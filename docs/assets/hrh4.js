// Copyright 2023 Aurora Operations, Inc.

// The delineation between h4 sections can be too subtle.  Add hr to make it stand out better.
Array.from(document.getElementsByTagName("H4"))
  .forEach(function(e, i) {
    while (e = e.nextElementSibling) {
      // Break at the next tag name.
      if (["H1", "H2", "H3", "H4"].includes(e.tagName)) {
        e.prepend(document.createElement("hr"));
        break;
      }
    }
  });
