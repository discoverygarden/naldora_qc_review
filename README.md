SUMMARY
-------

Performs Islandora Solr queries for Quality Control Review, 
and provides a form with checkboxes for each result. 
Allows to add filters to the Solr search. 

Currently, the results records are empty. 

The following workflow has been proposed through the QC Workflow User Stories:

1. The article's status ("Awaiting QC assignment") is set on ingest based on 
   the articles a fields in the article's journal membership. (NAL program)

2. QC manager
   Retrieves pending metadata for review and assigns to a reviewer.  
   The status on these records change to "awaiting QC review" (Story 8.1)

3. QC reviewer
   Brings up set of records assign to them for review (Story 8.3) in a 
   limited table review (Story 8.4). 
   The QC reviewer can click on the record and directly open the article's 
   MODS datastream in the NAL article MODS form for editing. (Story 8.5)
   The QC reviewer can mark the complete set of article for AI processing 
   (Story 8.6) or individual records within the set (Story 8.7). 
   Article status is set to "waiting".

The naldora_qc_review module addresses section 3: QC reviewer. 


REQUIREMENTS
------------

Islandora Solr and dependend modules and installations. 


INSTALLATION
------------


The following roles are created at module installation, if
not already present:

1. Naldora QC Manager
2. Naldora QC Reviewer

Both roles are granted the permissions 'use islandora_bookmark' and
'edit fedora metadata'. 

At uninstall, these roles are not removed from the Drupal database. 

Because the new roles are added with the next available weight, 
the Administrator role's weight should be manually adjusted 
(admin/people/permissions/roles). 

CONFIGURATION
-------------

No configuration options.

