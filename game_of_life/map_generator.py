#!/usr/bin/env python3

from random import choice
from argparse import ArgumentParser
import sys


def ntvdm():
    # по умолчанию поле 40 на 20, отношение живых клеток к мёртвым 1 к 2
    params = [int(e) for e in sys.argv[1:] + [40, 20, 1, 2]]
    width, height, alive, dead = params[:4]
    variants = [' ']*dead + ['#']*alive
    with open('generated_map.txt', 'wt') as f:
        for i in range(height):
            s = ''
            for j in range(width):
                s += choice(variants)
            f.write(s)
            f.write('\n')
    print('Field generated at file generated_map.txt')


if __name__ == '__main__':
    ntvdm()
