#!/usr/bin/env bun
import fs from 'fs'

// const filename = process.argv[2]
const std = fs.readFileSync(0, 'utf-8')

// const matches = std.match(/[0-9a-fA-F]:\s+([0-9a-fA-F]{2}\s*)+/g)

// if (!matches) process.exit(1)

const dict = {
    '0': '0000',
    '1': '0001',
    '2': '0010',
    '3': '0011',
    '4': '0100',
    '5': '0101',
    '6': '0110',
    '7': '0111',
    '8': '1000',
    '9': '1001',
    'a': '1010',
    'b': '1011',
    'c': '1100',
    'd': '1101',
    'e': '1110',
    'f': '1111'
}

console.log(std.replace(/[0-9a-fA-F]:\s+([0-9a-fA-F]{2}\s*)+/g, e => e.substring(0,2) + '    ' + e.substring(2).trim().replace(/[0-9a-fA-F]{2}/g, (g) => {
    return g.split('').map(e => dict[e]).join('')
}) + '    '))