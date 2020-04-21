;  This function reads a Laser_Prep.rsrc file, as transmitted from a
;  Macintosh, and converts it into an Ascii text PostScript file suitable
;  for manual downloading into a LaserWriter. If you have this file in 
;  its textual form, you can share a LaserWriter between a Unix system and
;  a Macintosh.
; 
;  For Gosling Emacs. It will work in Unipress Emacs if you fix the spelling
;  of "error-occured".
; 
;  Brian Reid, Stanford.
;  June 14, 1985
; 
(defun
    (get-16bit n nn
	(setq n (following-char))
	(if (< n 0) (setq n (+ n 256)))
	(forward-character)
	(setq nn (following-char))
	(if (< nn 0) (setq nn (+ nn 256)))
	(forward-character)
	(+ nn (* 256 n))
    )
    (cvt count stopflag ch v1 v2 v3
	 (visit-file "Laser_Prep.rsrc")
	 (error-occured (visit-file "LaserPrep.ps"))
	 (erase-buffer)
	 (pop-to-buffer "Laser_Prep.rsrc")
	 (setq stopflag 0)
	 (beginning-of-file)
	 (search-forward "0000000")
	 (search-reverse "0000000")
	 (backward-character)
	 (while (& (! (eobp)) (= stopflag 0))
		(setq count (following-char))
		(if (< count 0)
		    (setq stopflag 1)
		    (= count 0)
		    (progn
			  (sit-for 0)
			  (setq v1 (get-16bit))
			  (setq v2 (get-16bit))
			  (setq v3 (get-16bit))
			  (message v1 " " v2 " " v3) (sit-for 20)
			  (if (> v3 1024)
			      (setq stopflag 1))
		    )
		    (progn 
			   (forward-character)
			   (set-mark)
			   (provide-prefix-argument count
			       (forward-character)
			   )
			   (if (eobp) (setq stopflag 1))
			   (append-region-to-buffer "LaserPrep.ps")
			   (save-excursion 
			       (pop-to-buffer "LaserPrep.ps")
			       (end-of-file)
			       (insert-string "\n")
			   )
		    )
		)
	 )
	 (pop-to-buffer "LaserPrep.ps")
	 (end-of-file) (set-mark)
	 (sit-for 0)
	 (search-reverse "end") (search-forward "end")
	 (forward-character) (delete-to-killbuffer)
	 (sit-for 0)
	 (beginning-of-file)
	 (set-mark)
	 (search-forward "/md") (beginning-of-line)
	 (narrow-region) (beginning-of-file)
	 (while (! (eobp))
		(insert-string "%-")
gpa		(next-line) (beginning-of-line)
	 )
	 (widen-region)
	 (beginning-of-file)
	 (insert-string "%! Macintosh LaserWriter header file.\n")
	 (insert-string "% Converted from LaserPrep on ")
	 (insert-string (current-time))
	 (insert-string "\n")
    )
)
(message "Run the function 'cvt' to convert Laser_Prep.rsrc into LaserPrep.ps")
(sit-for 20)


