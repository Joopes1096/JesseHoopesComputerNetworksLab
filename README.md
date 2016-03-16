#Hi Luis!

#For this lab, I have gotten it to work for most cases with some few exceptions.  It works for a lot of cases with the sizes under 60 #bytes.  It runs when there is a message with one control character, a few control characters, and only control characters up to a #certain amount then it starts to act weird.  The checksum should work perfectly for the small file sizes.  When the larger file sizes #are introduced, it works with singularly placed control characters.  When back to back control characters are introduced, it starts to #act weird.  The amount of control characters was not an issue if they were not placed back to back.  All control characters didn't work. # The checksum should be working as well here.

#If you have any questions feel free to contact me!

#Jesse

#P.S.  Sometimes it seg faults on layer 2 on the receiving end but if you run it again it works fine without any change to the code.  #(It's weird I know)
