#!/usr/bin/env python
#
#
#
#   Copyright (C) Asiainfo, Inc.
#
# Copying and distribution of this file, with or without modification, are 
# permitted provided the copyright notice and this notice are preserved.
#
#
#

import commands
import cPickle
import time
import os
import sys
import socket
import getopt
import tempfile
import shutil
import smtplib,mimetypes
from email import Encoders
from email.MIMEBase import MIMEBase
from email.MIMEText import MIMEText
from email.MIMEMultipart import MIMEMultipart
#################################################################################
  
#################################################################################

#global var
gFatalError = ''
gPlatform = 32  #default will build 32bit lib
gBuildInterval = 7 # 7 days
gSendAnyway = False # send the log even there is not error during the build
gForceBuild = False # perform a force build even the src and build is up to date 
gFtp = False
gMakeCheck = True # do the make check or not
gCurSptPlat = ['HP-UX', 'AIX', 'SunOS', 'Linux'] #current suppported OS 
gPkgHome = os.getenv('HOME')
gOldLdLibPath = os.getenv('LD_LIBRARY_PATH', default='')
gWhoami = os.getenv('USER', default='Unknow')
gPwd = os.getcwd()
gLogFile = ''
sendMailTo = 'qianhf@asiainfo.com;hezk@asiainfo.com;fengzz@asiainfo.com;daizh@asiainfo.com'
mailToList = ('qianhf@asiainfo.com', 'hezk@asiainfo.com', 'fengzz@asiainfo.com', 'daizh@asiainfo.com')
sendMailFrom = 'aibc_automake@asiainfo.com'

gFtpUser = 'aibcdist'
gFtpPasswd = 'd1sta1bc'
gFtpHost = '10.3.2.242'


#set up the directory dictionary
AIBC_SRC_DIR_SHARE = gPkgHome+'/aibc/lib/' #srcdir is share between 32 and 64
AIBC_BUILD_DIR_32 = gPkgHome+'/aibclib_autobuild/lib_build_32/'
AIBC_INST_DIR_32 = gPkgHome+'/aibclib_autobuild/lib_inst_32/'
AIBC_BUILD_DIR_64 = gPkgHome+'/aibclib_autobuild/lib_build_64/'
AIBC_INST_DIR_64 = gPkgHome+'/aibclib_autobuild/lib_inst_64/'
 
AIBC_LOG_DIR_SHARE = gPkgHome+'/aibclib_autobuild/lib_log/' #logdir is share between 32 and 64
AIBC_LOG_DIR_SHARE_TODAY = AIBC_LOG_DIR_SHARE + time.strftime('%Y%m%d') + os.sep

AIBC_DIR_32 = {'builddir':AIBC_BUILD_DIR_32, 'instdir':AIBC_INST_DIR_32}
AIBC_DIR_64 = {'builddir':AIBC_BUILD_DIR_64, 'instdir':AIBC_INST_DIR_64}
AIBC_LIB_DIR = {'srcdir':AIBC_SRC_DIR_SHARE, '32':AIBC_DIR_32, '64':AIBC_DIR_64, 'logdir':AIBC_LOG_DIR_SHARE_TODAY}


#AIBC_DIR_SRC_LIST = [APL_SRC_DIR_SHARE, ACL_SRC_DIR_SHARE]
#AIBC_DIR_LOG_LIST = [APL_LOG_DIR_SHARE_TODAY, ACL_LOG_DIR_SHARE_TODAY]
#AIBC_DIR_32_LIST = [APL_BUILD_DIR_32, APL_INST_DIR_32, ACL_BUILD_DIR_32, ACL_INST_DIR_32]
#AIBC_DIR_64_LIST = [APL_BUILD_DIR_64, APL_INST_DIR_64, ACL_BUILD_DIR_64, ACL_INST_DIR_64]

#e.g, the directory can be got by AIBC_LIB_DIR['apl']['srcdir'] or
#AIBC_LIB_DIR['apl']['32']['builddir']

#file to store the latest build date
gLastBuildDateLog = AIBC_LOG_DIR_SHARE + '._aibc_lib_build_date'



# functions
#################################################################################


def rmDirContents(path):
    '''clean everything in the path'''

    for root, dirs, files in os.walk(path, topdown=False): 
        for name in files: 
            os.remove(os.path.join(root, name)) 
        for name in dirs: 
            os.rmdir(os.path.join(root, name))


def cleanDir(path):
    '''make a dir empty.'''
    if os.path.exists(path):
        rmDirContents(path)
    else: 
        os.makedirs(path)


def makeDirSane(mList):
    '''Clean all the dirs except log dirs.'''

    #clean the src directory except CVS direcotry
    cvsDir = AIBC_LIB_DIR['srcdir'] + os.sep + 'CVS'
    if os.path.exists(cvsDir):
        tempDir = tempfile.mkdtemp()
        shutil.rmtree(tempDir)
        shutil.move(cvsDir, tempDir)
        cleanDir(AIBC_LIB_DIR['srcdir'])
        shutil.move(tempDir, cvsDir)

    #check the log directory, creat it if not exist
    if not os.path.exists(AIBC_LIB_DIR['logdir']):
        os.makedirs(AIBC_LIB_DIR['logdir'])

    #clean the build and install directory
    if 32 in mList:
        for dir in AIBC_LIB_DIR['32']:
            cleanDir(AIBC_LIB_DIR['32'][dir])

    if 64 in mList:
        for dir in AIBC_LIB_DIR['64']:
            cleanDir(AIBC_LIB_DIR['64'][dir])

def logMsg(Msg):
    '''Log the message to the current log file.'''

    logFile = file(gLogFile, 'a')
    logFile.write(Msg+'\n')
    logFile.close()


def aibcCommand(aibcCmd): 
    '''Run a command and return the 2-tuple (status, output).'''
    
    statusOutput = commands.getstatusoutput(aibcCmd)
    return statusOutput 


def aibcLogCheckCommand(aibcCmd, expectStatus): 
    '''Run, log and check a command, then return the output.

    The ouptut of this command would be log and the status would be checked.
    if the status is not the expected status, a assert error is generated.'''
    logMsg(aibcCmd)

    statusOutput = aibcCommand(aibcCmd)

    logMsg(statusOutput[-1])
    if statusOutput[0] != expectStatus:
        logMsg('Error: *** ' + aibcCmd + ' failed !!!')
        assert False

    return statusOutput 


def envSetup(m):
    '''Set up the APL/ACL 32/64bit building environment.

    Set up environment for building and testing purpose.'''

    #32bit environment
    if 32 == m: 
        appendLdlibEnv = '/usr/local/lib:/usr/lib:/lib:'
        
    elif 64 == m:
        appendLdlibEnv = '/usr/local/lib/cppunit64/lib:/usr/local/lib:/usr/lib:/lib:'

    os.environ['LD_LIBRARY_PATH'] = appendLdlibEnv + gOldLdLibPath 


def srcCvsUp(srcPath):
    '''cvs update the APL/ACL src code.''' 

    if not os.path.exists(srcPath + os.sep + 'CVS'):
        errMsg = 'Make sure the source directory is updateable! CVS directory missed.'
        logMsg(errMsg)
        assert False
    else:
        cvsCmd = 'cvs up -Ad'

    logMsg('Changing dir to '+srcPath)
    os.chdir(srcPath)

    resp = aibcLogCheckCommand(cvsCmd, 0)

def buildLib(m):
    '''Configure and build the APL/ACL 32/64bit lib.

    one paramater is required in [32/64].'''

    global gMakeCheck

    logMsg('###############################################################################')
    logMsg('Strat to build ' + 'aibclib' + '_' + str(m) + ' library...')
    logMsg('###############################################################################')

    buildDir = AIBC_LIB_DIR[str(m)]['builddir']
    instDir = AIBC_LIB_DIR[str(m)]['instdir']
    srcDir = AIBC_LIB_DIR['srcdir']

    logMsg('Changing dir to '+buildDir)
    os.chdir(buildDir)

    prefix = '''"--prefix=''' + instDir + '''"'''

    if m == 64:
        mstr = ' -m64'
    else:
        mstr = ''

    libAutogen = srcDir + 'autogen.sh ' + mstr + ' -a' + prefix

    #configure the project thru the autogen script
    resp = aibcLogCheckCommand(libAutogen, 0)

    #make
    resp = aibcLogCheckCommand('make', 0)

    #make check
    if gMakeCheck == True:
        resp = aibcLogCheckCommand('make check', 0)

    #make install
    resp = aibcLogCheckCommand('make install', 0)


def compressLog(fileList):
    '''Compress the file in fileList to tar.gz format.'''

    for file in fileList:
        dirName = os.path.dirname(file) 
        fileName = os.path.basename(file)
        tarCmd = 'tar cvf - ' + fileName + '| gzip -c > ' + fileName + '.tar.gz'

        os.chdir(dirName)
        aibcLogCheckCommand(tarCmd, 0)

def makeMailMsg(logList):
    '''Build the content of mail.'''

    global gFatalError
    global gWhoami

    if len(gFatalError) != 0:
        mailMsg = 'The build is failed! Please check the log for detail.\r\n'
    else:
        mailMsg = 'The build is successful.\r\n'

    mailMsg = mailMsg + '\r\nThis is build by ' + gWhoami + ' with the command:\r\n"'
    for argv in sys.argv:
        mailMsg = mailMsg + argv + ' ' 
    mailMsg = mailMsg + '"\r\n\r\n'
    mailMsg = mailMsg + 'The log files in local disk are:\r\n'  
    for logfile in logList:
        mailMsg = mailMsg + logfile + '\r\n'
    mailMsg = mailMsg + '\r\n\r\n\r\n\r\n'
    mailMsg = mailMsg + '\r\n--------------------------------------------------------------------------------\r\nNote:\r\n'
    mailMsg = mailMsg + "This is send by aibc lib autobuild script, don't reply please!"

    return mailMsg


def sendLog(logList):
    '''Send out the mail with the build log.'''

    hostName = socket.gethostname()
    msg = MIMEMultipart() #creat a MIME object which including attachment
    msg['To'] = sendMailTo 
    msg['Subject'] ='AIBC lib autobuild log '+hostName+'_'+time.strftime('%Y%m%d')
    msg['From'] = sendMailFrom
    #build the content of mail
    mailMsg = makeMailMsg(logList)
    txt = MIMEText(mailMsg,_charset='utf-8')
    msg.attach(txt)

    for fileName in logList:
        fp = open(fileName,'rb')
        ctype,encoding = mimetypes.guess_type(fileName)
        if ctype is None or encoding is not None:
            ctype = 'application/octet-stream'
        maintype,subtype = ctype.split('/',1)
        m = MIMEBase(maintype,subtype)
        m.set_payload(fp.read())
        fp.close()
        Encoders.encode_base64(m) # encoding the attachment
        m.add_header('Content-disposition','attachment',filename=os.path.basename(fileName)) # modify the header
        msg.attach(m) # attach the file

    #FIXME
    #s = smtplib.SMTP('smtp.asiainfo.com')
    s = smtplib.SMTP('10.1.1.123',2025) 
    s.sendmail('qianhf@asiainfo.com', mailToList, msg.as_string())
    s.close()


def srcUpToDate():
    '''Check the CVS src tree updated or not via "cvs status" and "cvs up -Ad".'''

    #from the cvs manual, type 
    changes = ['F ', 'O ', 'E ', 'T ', 'C ', 'G ', 'U ', 'P ', 'W ', 'A ', 'M ', 'R ']
    cvsUpCmd = 'cvs up -Ad'
    cvsStatusCmd = 'cvs status'

    # treat it as out of date if CVS path not exist
    if not os.path.exists(AIBC_LIB_DIR['srcdir'] + os.sep + 'CVS'):
        return False

    os.chdir(AIBC_LIB_DIR['srcdir'])

    #cvs status check first
    statusOutput = aibcCommand(cvsStatusCmd)
    output = statusOutput[-1]
    outputlines = output.split(os.linesep)
    for line in outputlines:
        if 'Needs ' in line or 'Locally ' in line:
            return False

    #cvs update check again. 
    #Sometimes cvs up might return failed, e.g. merge failed during update
    #so ignore the return status here.
    statusOutput = aibcCommand(cvsUpCmd)
    output = statusOutput[-1]
    outputlines = output.split(os.linesep)
    for line in outputlines:
        for change in changes:
            if line.startswith(change):
                return False
    
    return True


def storeBuildDate():
    '''Store the build date in seconds format.'''
    #get the cur build date
    buildDate = time.time()
    f = file(gLastBuildDateLog, 'w')
    cPickle.dump(buildDate, f)
    f.close()


def loadBuildDate():
    '''Load the build date. 

    Return 0 if last build never happen or the builddate file is removed.'''
    #get last build date if exist
    if os.path.exists(gLastBuildDateLog):
        f = file(gLastBuildDateLog)
        return cPickle.load(f)
    else: 
        return 0


def rmBuildDate():
    '''Remove the build date if exists.'''

    if os.path.exists(gLastBuildDateLog): 
        os.remove(gLastBuildDateLog)


def buildUpToDate():
    '''Check the build updated or not.

    The build would treat as out of date after a time slice even the src 
    is not modified. The time slice is a week by default, user can change 
    it by specified the -d option.'''

    lastBuild = loadBuildDate()
    if lastBuild == 0:
        return False

    thisBuild = time.time()
    delta = thisBuild - lastBuild
    buildStaleTime = 3600 * 24 * gBuildInterval

    if delta > buildStaleTime:
        return False

    return True


def donotBuild():
    '''Check wether we should build'''
    
    if not gForceBuild and srcUpToDate() and buildUpToDate():
        print 'The build is up to date!'
        return True

    return False


def checkSupport():
    '''check what lib is support in the current platform.'''

    assert gPlatform in [0, 32, 64]

    if gPlatform == 0:
        spt = [32, 64]
    else:
        spt = [gPlatform]

    return spt


def buildAllLib():
    '''Build all the supported lib.

    Unless user specified the platform by -m option'''

    try: 
        supportPlatform = checkSupport()
    except:
        print 'Current platform or the specified lib is not supported!'
        sys.exit(2)

    #init 
    now = time.strftime('%H%M%S')
    logFileList = []
    global gFatalError

    #check any update in the CVS tree, or exit gently. 
    #go on unless user specified the force build option
    if donotBuild():
        print 'Build is up to date. Specify the -f option if you want to build anyway.'
        if gFtp :
            pkgNFtp()
        sys.exit()

    #make sure all the directory exists and all kinds of path are set
    makeDirSane(supportPlatform)

    curOS = commands.getoutput('uname')

    #build, check and log the libs.
    for sptPlt in supportPlatform: #go thru all the platforms

        global gLogFile
        envSetup(sptPlt)
        # Go thru the libs
        logDir = AIBC_LIB_DIR['logdir']
        gLogFile = logDir + 'aibclib' + '_' + curOS +'_'+ str(sptPlt) + '_' + now +'.log'
        logFileList.append(gLogFile)
        libSrcDir = AIBC_LIB_DIR['srcdir']
        try:
            srcCvsUp(libSrcDir)  #both 32bit and 64bit build share the same src
            buildLib(sptPlt)
        except:
            gFatalError = 'aibclib'
            print 'Fatal Error!!' 
            break;

    #compress the log file and remove the original file if successful
    try:
        compressLog(logFileList)
    except:
        print 'Warning: Compress log files failed!'
    #For saving the disk, remove the log file if tar successful.
    else:
        fileList = logFileList[:]
        for file in logFileList:
            fileList.remove(file)
            fileList.append(file+'.tar.gz')
            os.remove(file)
        logFileList = fileList

    #only send the log when user specified
    #if len(gFatalError) != 0 or gSendAnyway or not buildUpToDate():
    if gSendAnyway :
        sendLog(logFileList)

    if len(gFatalError):
        print 'Error: pls check the ', logFileList
        #remove build date if there is error
        rmBuildDate()
        sys.exit(2)

    #store the build date if no error 
    storeBuildDate()

def pkgNFtp():
    global gFtpUser
    global gFtpPasswd
    global gFtpHost
    global gPwd

    print "Packaging/Transfering files to ftp server", gFtpHost
    pkgScript = gPwd + "/aibc_package.sh"
    if not os.path.exists(pkgScript):
        print "pkg script: aibc_package.sh is not exist in current directory."
        return

    supportPlatform = checkSupport()
    srcDir = AIBC_LIB_DIR['srcdir']
    os.chdir(gPwd)

    for spt in supportPlatform :
        buildDir = AIBC_LIB_DIR[str(spt)]['builddir']
        instDir = AIBC_LIB_DIR[str(spt)]['instdir']
        pkgcmd = pkgScript + ' -s' + srcDir + ' -b' + buildDir + ' -i' + instDir + ' -u' + gFtpUser + ' -p' + gFtpPasswd + ' -h' + gFtpHost + ' -m' + str(spt) + ' >/dev/null'
        statusOutput = aibcCommand(pkgcmd)
	print statusOutput[-1]


#################################################################################


def main(argv):
    '''Build and test AIBC lib (apl acl anf) automatically.

    User can specified the -m option to build the 32 or 64 bit lib.
    This script designed to build the AIBC library project APL and ACL
    lib automatically. This includes the following processes:
        1. update the src code.
        2. build all the libraries in aibc/lib and run all the tests.
        3. send email to the specified receivers with the logs.
    To ensure the src in CVS tree is available, usally this script is
    called by cron daily.
    
    Options:
    -m   Platform [32|64|0]. By default, 32bit lib will be build. This 
         option sets the specified platform. 0 means both 32bit and 64bit.
    -s   By default, no log/email will be sent. By specified this options, 
         the log/mail will be sent.
    -f   By default, the library will not build if current build is up to date.
         Unless user specified this option to do a force build.
    -t   Package aibclib and transfer it to ftp server. By default, this is 
         disable, unless specified this option.
    -u   Don't run the test cases. (Disable 'make check' process)
    -d   specify the time slice (days) when a build will out of date even
         the src is not changed. By default, it is 7 days'''

    global gPlatform
    global gSendAnyway
    global gForceBuild
    global gFtp
    global gBuildInterval
    global gMakeCheck

    try:
        opts, args = getopt.getopt(argv, "hsfutm:d:", ["help", "send", "force", "uncheck", "ftp", "platform", "day"])
    except getopt.GetoptError:
        print main.__doc__
        sys.exit(2)
    for opt, arg in opts:
        if opt in ("-h", "--help"):
            print main.__doc__
            sys.exit()
        elif opt in ("-m", "--platform"):
            gPlatform = int(arg)
        elif opt in ("-s", "--send"):
            gSendAnyway = True
        elif opt in ("-f", "--force"):
            gForceBuild = True
        elif opt in ("-t", "--ftp"):
            gFtp = True
        elif opt in ("-u", "--uncheck"):
            gMakeCheck = False
        elif opt in ("-d", "--day"):
            gBuildInterval = int(arg)

    buildAllLib()

    if gFtp :
        pkgNFtp()


if __name__ == '__main__':

    main(sys.argv[1:])
