from ctypes import *
import re
mbase=windll.kernel32.GetModuleHandleA("injectconsole.dll".encode('ascii'))
getmoney=CDLL("injectconsole.dll").getmoney
getmoney.restype=None
getmoney.argtypes=[c_wchar_p,c_wchar_p]
def callbackfun(ebp,edi):
    try:
        print("wxid1111111111:",wstring_at(cast(ebp-0x220,POINTER(c_int)).contents.value))
        pwxid=c_wchar_p(cast(ebp-0x220,POINTER(c_int)).contents.value)
        msgtype=cast(ebp-0x230,POINTER(c_int)).contents.value
        print('mtype',msgtype)
        if msgtype==1:
            print("文字消息")
            print(wstring_at(cast(ebp-0x1F8,POINTER(c_int)).contents.value))
        elif msgtype==0x31:
            print("xml 消息")
            
            msg=wstring_at(cast(edi,POINTER(c_int)).contents.value)
            print(msg)
            tmp=re.findall(r'收到转账(\d+\.\d+)',msg)
            tid=re.findall(r'<transferid><!\[CDATA\[(\d+)\]\]></transferid>',msg)
            paytype=re.findall(r'<paysubtype>(\d)</paysubtype>',msg)
            if tmp and tid and paytype and paytype[0]=='1':
                tid=tid[0]
                money=float(tmp[0])
                ptid=(c_wchar_p)(tid)
                getmoney(ptid,pwxid)
				

    except Exception as e:
        print(e) 