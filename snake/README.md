CSCI 0300 Project 1 - Snake
===========================

## Design Overview:
Notable design choices:

The snake struct is defined as a direction, and two linked list one for the current row and another for the current column. 

There is a 0.4 seconds refresh time for the snake game 




## Collaborators: none

## Responsible Computing:
Partner CS login:

Q1:
The Unicode Consortium was founded by Joe Becker, Lee Collins, and Mark Davis. 
Major technology corporations, software producers, database vendors, governments, research institutions, and individual contributors are among current members. Current full members include Adobe, Airbnb, Amazon, Apple, Google, Meta, Microsoft, Salesforce, and Translated. 

Given they are mainly run by companies generally there goals will align with changes to help keep operations efficient and fit business needs. One positive aspect of this is cross platform compatibly between various services. 

Q2:

The Sunuwar script (used by the Sunuwar people in Nepal and India) is a example of a writing system used currently that remained digitally invisible. It was added to the Unicode 16.0 standard in September 2024. For the time prior to its addition to unicode, the lack of Unicode support forced the community to go to great lengths to preserve their written language in the city of Gangtok. One example of this is they produced an daily newspaper by hand, which was then duplicated via photocopy because no digital font or keyboard existed to type it. This absence of a digital representation creates a "digital divide" where native speakers cannot use their own alphabet for  modern tasks like texting, social media, or official record-keeping. When a language is missing from Unicode, it will suffer a loss of usage and utility in the world, making it harder for them to learn and use their heritages language. If the language is not on the web, it effectively ceases to exist in the eyes of the global community which can cause it to eventually fade away.

## Extra Credit attempted:

Grow more than one length on each food

Make a bot
    Create a simple bot that moves the snake in random directions! (3 points).
        uses random to pick a move direction
    Go beyond the simple bot — make it do :sparkles: better :sparkles: (3 points)
        uses calculates one move ahead to find the next move that gets the snake closer to the food.

Rather than rendering “S” characters, make a solid green snake [Hint: use Unicode characters like we do for the walls] (2 points).

To use the extra features refer to line 49-59 of the common.h file
Enable them by change the numbers to those listed in the comment on the same line

