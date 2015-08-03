Overview
========

This is TR1 test level for *pre-activated entities* case, triggered and antitriggered by either TRIGGER or SWITCH operations.

* Pre-activated entities are entities that has their *activation mask* set on design-time. They are immediately activated on level loading, using `InitialiseItem` routine in original source code.

Description
===========

* First (blue) tile is antitrigger.
* Second (dark yellow) tile is ordinary trigger.
* Third (beige) tile is switch trigger with only once flag set.
* Fourth (black) tile is switch trigger with only once flag AND timer set to 3.

Try to activate different tiles in different combinations, and compare the result in OpenTomb and TR1.