Project 5A - Concurrent Store
====================

<!-- TODO: Fill this out. -->

## Design Overview
none

## Collaborators
none

## Extra Credit Attempted

## Tweeter Reflection Questions

### Q1.

<!-- answer here!-->

I was assigned Pair #1 (Congressperson Kirby and the Freedom House Advocacy Group) and Pair #2 (Sarsra Breisand and Beth Abraham).

### Q2.

<!-- answer here!-->

I implemented an anonymization-based deletion strategy rather than complete removal of data. Instead of deleting all records associated with a user, my function replaces the user’s identifying information (their username) with a newly generated anonymous ID (e.g., "anon_user_00"). It then scans all posts and replies across the system and replaces occurrences of the user’s name with this anonymous identifier. This ensures that the content remains in the system, but it is no longer directly linked to the individual.

This approach balances the concerns of multiple stakeholders. The data subject (the user requesting deletion) has their identity effectively removed from the platform, satisfying the spirit of the GDPR “right to be forgotten.” At the same time, other users and the platform itself retain the integrity of conversations, posts, and reply structures. Fully deleting posts or replies could disrupt discussions or remove content that other users depend on, so anonymization minimizes harm to those stakeholders. Their opinions relating to her past can be relevant in the context of other politicians in a similar predicament, so even anonymized, it is still valuable. New users on the other hand will not necessarily find out this info belonged to her easily and will help reduce her old comments impact on her candidacy. I use case-insensitive substring replacement to ensure broader coverage of identifying information so that it will not miss a reference. This may over-replace in some cases. This tradeoff prioritizes stronger privacy protection over perfect textual accuracy. 

### Q3B. <!-- TODO: Change this header to reflect what version of Q3 you were assigned -->

<!-- answer here!-->

My implementation has several limitations. It only performs a single traversal of replies, meaning nested reply chains not linked to a user in all users may still contain identifying information. This is partly why I did not modify all_users as mutiple deletes would possibly leave reply from users not in the list and therefore missing from future gdprdeletes. The implmentation also uses direct substring matching, which can incorrectly replace parts of unrelated words (e.g., replacing “Ann” in “banana”). Additionally, it does not update all structural metadata (such as "all_users" or ownership relationships), which can leave the data store in an inconsistent state. The anonymization is also not fully irreversible, since I store a reverse mapping from the anonymous ID to the original user ID. 

To generalize this for all users, I would redesign the system to include structured metadata and indexing, such as explicitly tracking which posts and replies belong to each user and maintaining a tree of reply relationships. This would allow for recursive traversal of all related content instead of only 1 layer. I would also replace substring matching with boundary-aware matching (like checking if there a space ahead) to avoid incorrect replacements. Additionally, I would likely remove the reverse mapping to ensure stronger privacy guarantees and consider updating global structures like "all_users" to maintain consistency.

From the perspective of specific stakeholders, this generalized approach produces mixed results. For Sarsra Breisand, this approach is less effective. While anonymization removes her name, it does not guarantee that sensitive information like her location will stop spreading, especially if others continue reposting it. A full delete would be more effective here. Once another user know what her post originally was, just changing the name will not hide that they know its belongs to Sarsra. For Example, stakeholders like Beth Abraham may still retain access to anonymized data, which preserves research value but may conflict with the data subject’s intent to reduce attention entirely. Overall, while my implementation strikes a balance by reducing direct identification while preserving system integrity and some historical record, it does not fully satisfy all stakeholders, particularly in cases where complete removal or suppression of content is desired. Therefore, the generalized GDPRDelete() would still not fully satisfy all stakeholders, particularly those requiring complete removal of sensitive information.
