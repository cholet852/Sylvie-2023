from time import sleep

import serial

import argparse
import os

import torch
import torchaudio

from pyllamacpp.model import Model

from tortoise.api import TextToSpeech, MODELS_DIR
from tortoise.utils.audio import load_audio, load_voice, load_voices

from pydub import AudioSegment
from pydub.playback import play

ser = serial.Serial('/dev/ttyUSB0', 9600) # Establish the connection on a specific port

tts = TextToSpeech()
voice_samples, conditioning_latents = load_voice('sylvie', ['./voices'])
model = Model(ggml_model='./models/7B/ggml-alpaca-7b-q4-regen1.bin', n_ctx=512)

while True:
    user_prompt = input("Ask Sylvie to say something: ")

    ser.write(str(2).encode())
    sleep(0.10);
    ser.write(str(13).encode())
    sleep(0.30);
    ser.write(str(14).encode())
    sleep(0.20);
    ser.write(str(3).encode())
    
    gen_text = model.generate(user_prompt, n_predict=24, n_threads=12)

    ser.write(str(26).encode())


    print("Playing back the generated text:")
    print(gen_text)

    gen_audio = tts.tts_with_preset(gen_text, voice_samples=voice_samples, conditioning_latents=conditioning_latents, preset='ultra_fast')
    torchaudio.save('./results/sylvie_gen.wav', gen_audio.squeeze(0).cpu(), 24000)

    ser.write(str(25).encode())
    tts_audio = AudioSegment.from_wav('./results/sylvie_gen.wav')
    play(tts_audio)
