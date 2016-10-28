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

import os
import commands
import sys

#'''Check the CVS src tree updated or not via "cvs status" and "cvs up -Ad".'''

buildbot_master = '10.3.3.108:9989'
buildbot_cmd = '/DEVHome/aibcbuildbot/bin/buildbot'

print "checking src ..."

def aibcCommand(aibcCmd): 
    '''Run a command and return the 2-tuple (status, output).'''
    
    statusOutput = commands.getstatusoutput(aibcCmd)
    return statusOutput 

def sendChange(files, author='unknow'):
    #sendchange
    global buildbot_master
    changes = ''
    print 'author: ', author
    print 'sending changes to build master ...' 
    changes = ' '.join(files)
    cmd = buildbot_cmd + ' sendchange -u' + author + ' --master=' + buildbot_master + changes
    print 'the cmd is', cmd
    statusOutput = aibcCommand(cmd)
    print statusOutput[-1]

#from the cvs manual, type 
#changes = ['F ', 'O ', 'E ', 'T ', 'C ', 'G ', 'U ', 'P ', 'W ', 'A ', 'M ', 'R ']
changes = ['F ', 'O ', 'E ', 'T ', 'C ', 'G ', 'U ', 'P ', 'W ', 'A ', 'M ', 'R ']
cvsUpCmd = '/usr/local/bin/cvs up -Ad'
cvsStatusCmd = '/usr/local/bin/cvs status'
pkghome = os.getenv('HOME') 
aibcsrcdir = pkghome + '/aibc'

# treat it as out of date if CVS path not exist
if not os.path.exists(aibcsrcdir + os.sep + 'CVS'):
    sendChange(["Error in buildbot aibc src dir"])
    sys.exit(1)

os.chdir(aibcsrcdir)

#cvs status check first
#statusOutput = aibcCommand(cvsStatusCmd)
#output = statusOutput[-1]
#outputlines = output.split(os.linesep)
#statusFiles = []
#for line in outputlines:
    #if 'Needs ' in line or 'Locally ' in line:
        #elems = line.split()
        #statusFiles.append(elems[1])

#if len(statusFiles) != 0:
#    sendChange(statusFiles)
#    sys.exit(1)

#cvs update check again. 
#Sometimes cvs up might return failed, e.g. merge failed during update
#so ignore the return status here.
changeFiles = []
statusOutput = aibcCommand(cvsUpCmd)
output = statusOutput[-1]
outputlines = output.split(os.linesep)
print output
for line in outputlines:
    for change in changes:
        if line.startswith(change):
            elems = line.split()
            changeFiles.append(elems[1])

if len(changeFiles) != 0:
    author_list = [] 
    for f in changeFiles:
        cmd = '/usr/local/bin/cvs log ' + f
        statusOutput = aibcCommand(cmd)
        output = statusOutput[-1]
        outputlines = output.split(os.linesep)
        for line in outputlines:
            if line.startswith('date: '):
                start = line.find('author: ')
                end = line[start:].find(';') 
                author = line[start+8 : start+end]
                if not author in author_list:
                    author_list.append(author)
                break
        #only check the first changed file's author
        break
    if not author_list:
        author = 'unknow'
    else:
        author = "+".join(author_list)
    sendChange(changeFiles, author)
    sys.exit(1)

print "check src done"
sys.exit(0)
