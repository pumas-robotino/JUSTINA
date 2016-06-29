import threading
import BB

class ParallelSender(object):
    '''
    Sends a command and waits for the answer in parallel to other thread's execution,
    allowing other thread's to poll if the response have been received.
    
    :param Command command: Command to be sent, must be an instance of class Command.
    :param int timeout: (Default 300000) How much time (in miliseconds) to wait for response before trying again or aborting.
    :param int attempts: (Default 1) How many attempts to send the command if no response is received after timeout.
                If attempts is 0, it will keep trying indefinitely until StopSending is called. (Use carefully)
    
    .. note::
    
        Notice the command is sent when the object is created.
    
    '''

    def __init__(self, command, timeout = 300000, attempts = 1):
        self._sendingLock = threading.Lock()
        self._sending = True
        
        self._respLock = threading.Lock()
        self._response = None
    
        self._command = command
        
        self._attemptsLock = threading.Lock()
        self._attempts = attempts
        
        self._timeout = timeout/1000.0
        
        self._p = threading.Thread(target=self._Execute)
        self._p.daemon = True
        self._p.start()
        
    @property
    def sending(self):
        '''
            A property that indicates whether the object is still waiting for a response.
        '''
        self._sendingLock.acquire()
        r = self._sending
        self._sendingLock.release()
        return r
    
    def _setSending(self, s):
        self._sendingLock.acquire()
        self._sending = s
        self._sendingLock.release()
    
    @property
    def response(self):
        '''
            A property for retrieving the response object generated by the command.
            
            This property should be used when *sending* is ``False``.
        '''
        if not self._respLock.acquire(False):
            return None
        r = self._response
        self._respLock.release()
        return r
    
    def _setResponse(self, R):
        self._respLock.acquire()
        self._response = R
        self._respLock.release()
    
    def StopSending(self):
        self._attemptsLock.acquire()
        self._attempts = 1
        self._attemptsLock.release()
        
    def _Execute(self):
        
        response = None
        
        currentAttempt = 0
        
        self._attemptsLock.acquire()
        att = self._attempts
        self._attemptsLock.release()
        
        while not response and (att == 0 or currentAttempt < att):
            currentAttempt += 1
            response = BB.SendAndWait(self._command, self._timeout)
            
            self._attemptsLock.acquire()
            att = self._attempts
            self._attemptsLock.release()
        
        self._setResponse(response)
        self._setSending(False)
        