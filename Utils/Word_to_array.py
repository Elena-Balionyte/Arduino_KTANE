import string
while True:
    out_string = ''
    word = input("word:").lower()
    #freq = input("freqency:")
    #out_string += f"{len(word)},"
    #out_string += f"{freq},"
    #for letter in word:
    #    out_string += f'{string.ascii_lowercase.index(letter)},'
    #out_string += "},"
    #print(out_string.replace(',}','}'))
    for letter in word:
        out_string += f'{string.ascii_lowercase.index(letter)};'
    print(out_string)
