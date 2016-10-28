# -*- coding: UTF-8 -*-
import sys
import optparse

def easyMain(optParser, mainFunc, *argv, **kwargs):
    optGroup = optparse.OptionGroup(optParser, "pyailib.utils.easyMain Options:", "these options would be enable if you're using pyailib.utils.easyMain to run your program") 
    optGroup.add_option("-O", "--psyco-full",
            dest="_psycoFull", 
            action="store_true", 
            help="using 'psyco.full()' to speed-up program",
    )
    optGroup.add_option("-L", "--log-config",       
            dest="_logConfig", 
            metavar="LOG_CONFIG", 
            help="using 'logging.basicConfig' to initialize global logger",
    )
    optGroup.add_option("-P", "--profile",
            dest="_profStatDepth", 
            metavar="STAT_DEPTH", 
            type="int", 
            default=0, 
            help="using 'hotshot' to profile program",
    )
    optGroup.add_option("-S", "--twisted-logging",  
            dest="_twistedLogging", 
            action="store_true", 
            help="foward 'twisted.python.log' messages to 'logging module'",
    )
    optGroup.add_option("-R", "--twisted-reactor",  
            dest="_twistedReactor",
            metavar="REACTOR_TYPE",
            help="install 'twisted.internet.<REACTOR_TYPE>reactor' as default reactor",
    )   
    optParser.add_option_group(optGroup)

    opts, args = optParser.parse_args()
    
    # psyco.full()
    if hasattr(opts, "_psycoFull") and opts._psycoFull:
        try:
            import psyco
            psyco.full()
        except Exception, e:
            print >> sys.stderr, "error handling -O/--psyco-full (%r), skip!" % e
    
    # logging.basicConfig()
    if hasattr(opts, "_logConfig") and isinstance(opts._logConfig, str):
        try:
            import logging
            kw = eval("{" + opts._logConfig + "}")
            assert(isinstance(kw, dict))
            logging.basicConfig(**kw)
        except Exception, e:
            print >> sys.stderr, "error handling -L/--log-config (%r), skip!" % e
    
    # hotspot
    if hasattr(opts, "_profStatDepth") and opts._profStatDepth > 0:
        try:
            def profMain(func):
                def _mainWrapper(*argv, **kwargs):
                    import hotshot, hotshot.stats
                    profFile = "%s.prof" % sys.argv[0]
                    prof = hotshot.Profile(profFile)
                    print >> sys.stderr, "start profiling..."
                    prof.runcall(func, *argv, **kwargs)
                    prof.close()
                    print >> sys.stderr, "finish profiling, now stating, need some minuts..."
                    stats = hotshot.stats.load(profFile)
                    stats.strip_dirs()
                    stats.sort_stats('time', 'calls')
                    stats.print_stats(opts._profStatDepth)
                return _mainWrapper
            mainFunc = profMain(mainFunc)
        except Exception, e:
            print >> sys.stderr, "error handling -P/--profile (%r), skip!" % e
    
    # twisted.python.log
    if hasattr(opts, "_twistedLogging") and opts._twistedLogging:
        try:
            from twisted.python import log
            observer = log.PythonLoggingObserver()
            observer.start()
        except Exception, e:
            print >> sys.stderr, "error handling -S/-twisted-logging (%r), skip!" % e
    
    # twisted.internet.*reactor
    if hasattr(opts, "_twistedReactor") and opts._twistedReactor:
        try:
            from pyailib.utils import importModule
            r = importModule(opts._twistedReactor)
            r.install()
        except Exception, e:
            print >> sys.stderr, "error handleing -R/--twisted-reactor (%r), skip!" % e

    return mainFunc(optParser, *argv, **kwargs)

