split_rkimage: split_rkimage.c
	gcc -o $@ $^
clean:
	rm split_rkimage
