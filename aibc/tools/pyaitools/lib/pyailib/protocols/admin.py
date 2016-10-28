from twisted.manhole.telnet import ShellFactory
from twisted.application import strports

    
def makeAdminServer(options, namespace={}):
    if options["admin-port"]:
        factory = ShellFactory()
        factory.username = options['admin-username']
        factory.password = options['admin-password']
        factory.namespace.update(namespace)
        s = strports.service(options["admin-port"], factory)
        factory.setService(s)
        return s
    else:
        return None
        
        