CSCI 0300 Project 1 - Snake
===========================

## Design Overview:
Notable design choices:

The snake struct is defined as a direction, and two linked list one for the current row and another for the current column. The first item in the list is the head of the snake.

There is a 0.4 seconds refresh time for the snake game 

g_EC is also used to function as a easy way to turn the extra credit items on and off without changing the setting themselves. It may seem a bit redundant but it was made more for convenience of testing. 

All other choice follow the assignment specified in https://csci0300.github.io/assign/projects/project1.html


## Collaborators: none

## Responsible Computing:
Partner CS login: ucho2

Q1:
The Unicode Consortium was founded by Joe Becker, Lee Collins, and Mark Davis. 
Major technology corporations, software producers, database vendors, governments, research institutions, and individual contributors are among current members. Current full members include Adobe, Airbnb, Amazon, Apple, Google, Meta, Microsoft, Salesforce, and Translated. 

Given they are mainly run by companies generally there goals will align with changes to help keep operations efficient and fit business needs. One positive aspect of this is cross platform compatibly between various services. 

Q2:

The Sunuwar script (used by the Sunuwar people in Nepal and India) is a example of a writing system used currently that remained digitally invisible. It was added to the Unicode 16.0 standard in September 2024. For the time prior to its addition to unicode, the lack of Unicode support forced the community to go to great lengths to preserve their written language in the city of Gangtok. One example of this is they produced an daily newspaper by hand, which was then duplicated via photocopy because no digital font or keyboard existed to type it. This absence of a digital representation creates a "digital divide" where native speakers cannot use their own alphabet for  modern tasks like texting, social media, or official record-keeping. When a language is missing from Unicode, it will suffer a loss of usage and utility in the world, making it harder for them to learn and use their heritages language. If the language is not on the web, it effectively ceases to exist in the eyes of the global community which can cause it to eventually fade away.

3.A

There were multiple reasons behind the effort against Han Unification. A major point, leaning more on the technical side of the movement, was its choice to have a single code point represent characters across languages. While a character may have a similar look, they are not always the same in their respective languages, so it was argued that this was a reason for assigning different code points. Another point was that the actual rendering of the characters was handled by fonts for each language. The main worry about this was that if a text used the wrong font, it could render Japanese text in a Chinese style.

Less on the technical side, there was also a cultural consideration. The idea of combining language representation, while efficient from a technical standpoint, was seen by some as a muddling of cultures. Some believed this would lead to their national typographic traditions being overridden.



3.B
Reasons for: 
characters represented in a standard way that’s accessible for all types of devices
simplifying the character representation in unicode standard
Reasons against: 
Character that can be represented by the same code point are not always the same so they should get their own representation
It was worried that the integration would result in errors when rendering multilingual text
Final agreement
Modern day implementation addresses a lot of the downsides of han unification
Makes it possible for many/all devices to display/understand these languages
Good to have Han unification in the long term


## Extra Credit attempted:

Grow more than one length on each food. Enable in common.h.
    Allows snake to grow n times per food eaten 

Make a bot
    Create a simple bot that moves the snake in random directions! (3 points).
        uses random to pick a move direction
    Go beyond the simple bot — make it do :sparkles: better :sparkles: (3 points)
        calculates one move ahead to find the next move that gets the snake closer to the food.

Rather than rendering “S” characters, make a solid green snake [Hint: use Unicode characters like we do for the walls] (2 points).

To use the extra features refer to line 49-59 of the common.h file
Enable them by change the numbers to those listed in the comment on the same line

