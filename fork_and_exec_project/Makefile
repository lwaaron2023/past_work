all: 
	make prolific
	make generation
	make explorer
	make slug
	make slugrace

prolific: prolific.c
	gcc -Wall prolific.c -o prolific

generation: generation.c
	gcc -Wall generation.c -o generation

explorer: explorer.c
	gcc -Wall explorer.c -o explorer

slug: slug.c
	gcc -Wall slug.c -o slug

slugrace: slugrace.c
	gcc -Wall slugrace.c -o slugrace


clean:
	rm prolific
	rm generation
	rm explorer
	rm slug
	rm slugrace