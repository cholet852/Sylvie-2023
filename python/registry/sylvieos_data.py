from time import sleep
import pyfiglet
import os

version = "2023 (2.0)"

def splash():
    ascii_banner = pyfiglet.figlet_format("Sylvie OS")
    print(ascii_banner)
    print("Written by Denise Jaimes, Supreme Leader")
    print("New Oceania Military Academy")
    print("Version : " + version)
    
    sleep(3)
    
    os.system('clear')
