01 WIDTH 40 : CLS
02 READ S$
03 IF S$ = "*" THEN 06
04 PRINT S$;
05 GOTO 02
06 PRINT : PRINT : INPUT "REPEAT LAST SENTENCE (Y/n)";S$
07 IF S$ = "Y" OR S$ = "y" THEN RESTORE 455 : PRINT : GOTO 02
08 END
20 DATA "Lorem ipsum dolor sit amet, consectetur "
21 DATA "adipiscing elit, sed do eiusmod tempor "
22 DATA "incididunt ut labore et dolore magna "
23 DATA "aliqua. Ut enim ad minim veniam, quis "
24 DATA "nostrud exercitation ullamco laboris "
25 DATA "nisi ut aliquip ex ea commodo consequat. "
26 DATA "Duis aute irure dolor in reprehenderit "
27 DATA "in voluptate velit esse cillum dolore "
28 DATA "eu fugiat nulla pariatur. Excepteur sint "
29 DATA "occaecat cupidatat non proident, sunt in "
30 DATA "culpa qui officia deserunt mollit anim id "
31 DATA "est laborum. "
32 DATA "Sed ut perspiciatis, unde omnis iste natus "
33 DATA "error sit voluptatem accusantium doloremque "
34 DATA "laudantium, totam rem aperiam eaque ipsa, "
35 DATA "quae ab illo inventore veritatis et quasi "
36 DATA "architecto beatae vitae dicta sunt, "
37 DATA "explicabo. Nemo enim ipsam voluptatem, "
38 DATA "quia voluptas sit, aspernatur aut odit "
39 DATA "aut fugit, sed quia consequuntur magni "
40 DATA "dolores eos, qui ratione voluptatem sequi "
41 DATA "nesciunt, neque porro quisquam est, qui do "
42 DATA "lorem ipsum, quia dolor sit amet consectetur "
43 DATA "adipisci[ng] velit, sed quia non numquam [do] "
44 DATA "eius modi tempora inci[di]dunt, ut labore et "
45 DATA "dolore magnam aliquam quaerat voluptatem. "
46 DATA "Ut enim ad minima veniam, quis nostrum[d] "
47 DATA "exercitationem ullam corporis suscipit "
48 DATA "laboriosam, nisi ut aliquid ex ea commodi "
49 DATA "consequatur? [D]Quis autem vel eum iure "
50 DATA "reprehenderit, qui in ea voluptate velit "
51 DATA "esse, quam nihil molestiae consequatur, "
52 DATA "vel illum, qui dolorem eum fugiat, quo "
53 DATA "voluptas nulla pariatur? [33] At vero eos et "
54 DATA "accusamus et iusto odio dignissimos ducimus, "
55 DATA "qui blanditiis praesentium voluptatum "
56 DATA "deleniti atque corrupti, quos dolores et "
57 DATA "quas molestias excepturi sint, obcaecati "
58 DATA "cupiditate non provident, similique sunt in "
59 DATA "culpa, qui officia deserunt mollitia animi, "
60 DATA "id est laborum et dolorum fuga. Et harum "
61 DATA "quidem rerum facilis est et expedita "
62 DATA "distinctio. Nam libero tempore, cum soluta "
63 DATA "nobis est eligendi optio, cumque nihil "
64 DATA "impedit, quo minus id, quod maxime placeat, "
65 DATA "facere possimus, omnis voluptas assumenda "
66 DATA "est, omnis dolor repellendus. Temporibus "
67 DATA "autem quibusdam et aut officiis debitis aut "
68 DATA "rerum necessitatibus saepe eveniet, ut et "
69 DATA "voluptates repudiandae sint et molestiae non "
70 DATA "recusandae. Itaque earum rerum hic tenetur a "
71 DATA "sapiente delectus, ut aut reiciendis "
72 DATA "voluptatibus maiores alias consequatur aut "
73 DATA "perferendis doloribus asperiores repellat."
74 DATA "dignitate ecpetendum est, eorum autem quae sunt "
75 DATA "prima naturae propter se nihil est expetendum. "
76 DATA "Cum vero ilia quae oflicia esse dixi proficiscantur ab "
77 DATA "initiis naturaej necesse est ea ad haec referri, ut "
78 DATA "recte dici possit omnia oflicia eo referri ut adipisca- "
79 DATA "mur principia naturae, nee tamen ut hoc sit bonorum "
80 DATA "ultimiim, propterea quod non inest in primis naturae "
81 DATA "conciliationibus honesta actio; consequens est enim "
82 DATA "et post oritur, ut dixi. Est tamen ea secundum "
83 DATA "naturam multoque nos ad ae espetendam magis hor- "
84 DATA "tatur quam superiora omnia. Sed e hoc primum "
85 DATA "error tollendus est, ne quis seqiii existimet ut duo "
86 DATA "sint ultima bononim. Ut enim si cui propoaitum sit "
87 DATA "collineare hostora aliquo aut sagittam, sicut ' nos "
88 DATA "ultimum in bonis didmus, sic illi facere omnia quae "
89 DATA "possit ut collineet: huic in eiusmodi similitudine "
90 DATA "omnia sint facienda ut collineet, et tamen, ut omnia "
91 DATA "faciat quo propositum assequatur, sit* hoc quasi ulti- "
92 DATA "mum quale nos summum in vita bonum dtcimus, illud "
93 DATA "autem ut feriat, quasi seligendum, non espetendum. "
94 DATA "Cum autem omnia oflicia a principiis naturae "
95 DATA "proliciscantur, ab iisdem necesse est proficisci ipsam "
96 DATA "sapientiam. Sed quemadmodum saepe fit ut is qui "
97 DATA "commendatus sit alieui pluris cum faciat cui com- "
98 DATA "mendatus quam ilium a quo sit, sic minime mirum "
99 DATA "itemque et ratio et perfecta ratio. Ut enim "
100 DATA "histrioni actio, saltatori motus non quivis sed certus "
101 DATA "quidam est datus, sic vita agenda est certo Renere "
102 DATA "quodam, non quolibet; quod genus conveniens con- "
103 DATA "sentaneumque dicimus. Nee enim gubemationi aut "
104 DATA "niedicinae similem sapientiam esse arbitramur, sed "
105 DATA "actioni illi potius quam modo dixi et saltAtioni, ut in "
106 DATA "ipsa insit, non tons petatur extremum, id est artis "
107 DATA "effectio. Et tamen est etiam alia cum his ipsis "
108 DATA "artibus sapientiae dissimilitude, propterea quod in "
109 DATA "illis quae recte facta sunt non continent tamen "
110 DATA "omnes partes e quibus constant; quae autem nos aut "
111 DATA "recta aut recte facta dicamus, si placet, illi autem "
112 DATA "appellant, omnes numeros virtutis continent "
113 DATA "Sola enim sapientia in setotaconversaest, quod "
114 DATA "Sed cum guod lionestum sit id solum bonum esse "
115 DATA "dicamus, consentaneum tanien est fungi officio cum "
116 DATA "id otficium nee in bonis ponamus nee in mails. Est "
117 DATA "enim aliquid in his rebus probabile, et quidem ita ot "
118 DATA "eius ratio reddi possit; ergo ut etiam probabiliter "
119 DATA "acti ratio reddi possit; est autem oHiciiim quod ita "
120 DATA "factum est ut eius facti probabilis ratio reddi possit ; "
121 DATA "ex quo intellegitur officium medium quiddam esse "
122 DATA "quod nequc ill bonis ponatur neque iu contrariis. "
123 DATA "Quoniamque in iis rebus quae neque in virlutibus "
124 DATA "sunt neque in vitiis, est tanicn quiddam quod usui "
125 DATA "possit esse, tolleiidum id non est. Est auteni eius "
126 DATA "generis actio quoque quaedam, et quidem talis at "
127 DATA "ratio postulet agere aliquid et facere eorura; quod "
128 DATA "autem ratione actum est id ofRcium appellamus; est "
129 DATA "igitur ofiicium eius generis quod nee in bonis ponatur "
130 DATA "nee in contrariis. "
131 DATA "Quoniam enim videmUs esse quiddam quod reete "
132 DATA "factum appellemus, id autem eat perfectum otficium, "
133 DATA "erit etiam iiichoutum ; ut, si iiiste deposituin reddere "
134 DATA "in recte faclis sit, in officiis ponatur deposituin red- "
135 DATA "dere ; illo enim addito ' iuste/ fit recte factum, per se "
136 DATA "autem hoc ipsum reddere in oHicio ponitur. Quoiii- "
137 DATA "amque non dubium est quin in iis quae media dica- "
138 DATA "mus sit aliud sumendiim, aliud reieieudum, quidquid "
139 DATA "ita lit nut dicitur omne officio continetur. Ex quo "
140 DATA "intellegitur quoniam se ipsi omnes natura diligant, "
141 DATA "tarn insipientera qunm sapientem sumpturum quae "
142 DATA "secundum naturam sint reierturumque contraria. Ita "
143 DATA "est quoddam commune ofiieium sapientis et insi- "
144 DATA "pientis ; ex quo efficitur versari in iis quae media "
145 DATA "60 dicamus. Sed cum ab liis omnia proliciscantur "
146 DATA "officia, non sine causa dicitur ad ea rcferri omnes "
147 DATA "nostras cogitationes. in his et excessum e vita et in "
148 DATA "rita mansionem. In quo enim plura sunt quae "
149 DATA "secundum uaturam sunt, huius officium est in vita "
150 DATA "manere; in quo autem aut sunt plura contraria aut "
151 DATA "fore videntur, huius oificium est e vita excedere, "
152 DATA "Nam bonum illud et malum quod "
153 DATA "saepe iam dictum est postea consequitur ; prima autem "
154 DATA "ilia naturae sive secunda sive contraria sub judicium "
155 DATA "sapientis et dilectum caduiit, estque ilia subieeta "
156 DATA "quasi materia sapientiae. Itaque et manendi in vita "
157 DATA "et migrandi ratio Dnmis iis rebus quas supra dixi "
158 DATA "metienda. Nam neque virtute retinetur in vita, "
159 DATA "nee iis qui sine virtute sunt mors est oppetenda. Et "
160 DATA "saepe officium est sapientis desciscere a vita cum sit" 
161 DATA "beatjssimus, si id opportune facere possit. Sic enira "
162 DATA "censent, opportunitatis esse beate vivere quod est "
163 DATA "convenienter naturae vivere.' Itaque a sapientia "
164 DATA "praecipitur se ipsam si usus sit sapiens ut relinquat. "
165 DATA "Quamobrem cum vitiorum ista vis non sit ut causam "
166 DATA "alferant mortis voluntariae, perspicuum est etiam "
167 DATA "stultorum qui iideni miseri sint officium esse manere "
168 DATA "in vita, si sint in maiore parte earum rerura quas "
169 DATA "secundum naturam esse dicimus, Et quoniam exce- "
170 DATA "dens e vita et manens aeque miser est, nee diutumttas "
171 DATA "magis ei vitam fugiendam facit, non sine causa dicitur "
172 DATA "iis qui pluribus naturalibus frui possint esse in vita "
173 DATA "manendum. "
174 DATA "Mundum autem censent regi numine deomim "
175 DATA "eumque esse quasi communeta urbem et civitatem "
176 DATA "liominum et deoruiOj et imumquemque nostnim eius "
177 DATA "mundi esse partem; ex quo illud natura consequl ut" 
178 DATA "communem utilitatem nostrae anteponamus. Ut "
179 DATA "eniin leges omnium salutem siiigularum saluti ante- "
180 DATA "ponunt, sic vir bonus et sapiens et legibus parens et" 
181 DATA "civilis oflici non ignarus utilitati omnium plus quam "
182 DATA "unius alicuius aut suae consulit Nee magis est vitu- "
183 DATA "perandus proditor patriae quam communis utilitatis "
184 DATA "aut safutis deserter propter suam utilitatem aut "
185 DATA "salutem. Ex quo fit ut laudaiidus is sit qui mortem "
186 DATA "oppetat pro re publica, quod defeat cariovem nobis "
187 DATA "esse patriam quam nosmet ipsos. Quoniamque ilia "
188 DATA "VOX inhuiuana et scelerata ducitur eorum qui iiegant "
189 DATA "se recusare quo minus ipsis mortuis terrarum omnium "
190 DATA "deflagratjo cousequatur (quod vulgari quodam versu "
191 DATA "Graeco pronuntiari solet), certe verum est etiam iis "
192 DATA "qui aliquaudo futuri sint esse propter ipsos con- "
193 DATA "sul endum. "
194 DATA "quam plurimis iu primisque docendo rationibusque "
195 DATA "pmdentiae tradendis. Itaijue non facile est iiivenire "
196 DATA "qui quod sciat ipse non tradat alteri ; ita non solum "
197 DATA "ad discendum propensi suraus verum etiam ad docen- "
198 DATA "duni. Atque ut tauris natura datum est ut pro vi- "
199 DATA "tulis contra leones summa vi impetuque contendant, "
200 DATA "sic ii qui valent opibus atque id facere possunt, ut de "
201 DATA "Hercule et de Libero accepimus, ad servandura genus "
202 DATA "hominum natura incitaiitur. Atque etiam lovem "
203 DATA "cum Optimum et Maximum dicimus cumque eundem "
204 DATA "SaFutarem, Hospitalem, Statorem, hoc intellegi volu- "
205 DATA "mus, salutem hominum in eius esse tutela. Minime "
206 DATA "autem convenitj cum ipsi inter nos viles neglectique "
207 DATA "simus, postulare ut dis immortalibus cari simus et ab "
208 DATA "iis dihgamur. Quemadmodum igitur membris uti- "
209 DATA "mur prius quam didicimus cuius ea utilitatis causa "
210 DATA "habeamus, sic inter nos natura ad civilem communi- "
211 DATA "tatem coniuncti et consociati sumus. Quod ni ita se "
212 DATA "haberet, nee iustitiae ullus esset nee bonitati loous."
213 DATA "Amicitiam autem adhibendam esse censent quia "
214 DATA "sit ex eo genere quae prosunt. Quamquam autem "
215 DATA "in amicitia alii dicant aeque caram esse sapienti "
216 DATA "rationeni amici ac suam, alii autem sibi cuique "
217 DATA "cariorem suam, tamen hi quoque posteriores fatentur "
218 DATA "alienum esse a iustitia, ad quam nati esse videamur, "
219 DATA "detrahere quid de aliquo quod sibi iissumat. Minime "
220 DATA "vero probatur huic disciplinae de qua loquor aut "
221 DATA "iustitiam aut amicitiam propter utilitates ascisci aut "
222 DATA "probari. Eaedem enim utilitates poterunt eas labe- "
223 DATA "factarc atque pervertere. Etenim nee iustitia nee "
224 DATA "amicitia esse omnino poterunt nisi ipsae per se"
225 DATA "quae notae illustresque suntj iustitiam dico, tempe- "
226 DATA "rantiam, ceteras generis eiusdem (quae omnes similes "
227 DATA "artium reliquarum materia tantum ad meliorem "
228 DATA "partem et trnctationc differunt), casque ipsas virtutes "
229 DATA "viderent nos itiagnificentius appetere et ardentius: "
230 DATA "habere etiam iiisitani quandam vel potius innatam "
231 DATA "cupiditutetn seientiae, natosque esse ad coiigreja- "
232 DATA "tionem hominum et ad sucietatem com muni tatemque "
233 DATA "generis humani, eaque in maKiinis ingeniis niaxinie" 
234 DATA "elucere, totatn philosophiam tres in partes diviserunt, "
235 DATA "quam partitionem a Zenone esse retentam videmus. "
236 DATA "J Quarum cuoi una sit qua mprg s confor mari putantur, "
237 DATA "difFero earn partem, quae quasi stirps est huius "
238 DATA "quaestionis ; qui sit enim finis bonorum, mox ; lioc "
239 DATA "loco tantum dico a veteribus Peripateticis Aeademi- "
240 DATA "cisque, qui re consentientes vocabulis differebant, "
241 DATA "eum locum quern civilem recte appellaturi videmur "
242 DATA "oratio. Deinde ea quae requirebant orationem "
243 DATA "ornatiim et gravem, quam magnifice stmt dicta ab "
244 DATA "illis, quam spleiidide! de iustitia, de temperantia, de "
245 DATA "fortitudine, de aniicitia, de aetate degeiida, de plii- "
246 DATA "losophia, de capessenda re piiblica, hominum nec "
247 DATA "spinas vellentium, ul Stoici, nec ossa nudantium, "
248 DATA "sed eorum qui gratidin ornate vellent, enucleate "
249 DATA "minora dicere. Itaque quae sunt eorum consola- "
250 DATA "tiones, quae coliortationes, quae etiam monita et "
251 DATA "consilia scripta ad summos viros ! Erat enim spud "
252 DATA "COS, ut est reruni ipsarum natura, sic dicendi exerci- "
253 DATA "tatio duplex. Nam quidiiuid quaeritur, id habet aut "
254 DATA "generis ipsius sine personis teuiporib usque aut iis "
255 DATA "adiunctis facti aut iuris aut iioniinis controversiam." 
256 DATA "Ergo in utroque exercebantur ; eaque disciplina "
257 DATA "effeeit lantum illorum utroque in genere dicendi "
258 DATA "stinguet citius si ardentem acceperit. Ista i "
259 DATA "quae tu breviter, regem, dictatorem, divitem sol "
260 DATA "esse sapienteni, a te quid em aptc ac rotund) "
261 DATA "quippe; iiabes enim a rhetorilms; illorum "
262 DATA "ipsa quam exsiUa de virtutis vil quam tantam volunt "
263 DATA "esse ut lieatum per se efficere possit. Pungunt "
264 DATA "enim, quasi aculeis, interrogatiunculis angustis, "
265 DATA "quibus etiam qui assentiuntur nihil coounutantur "
266 DATA "animo et iidem abeunt qui venerant; res enim for"
267 DATA "tasse verae, eerte graves, noii ita tractantur ut" 
268 DATA "debentj sed aliquanto minutius. "
269 DATA "Sequiturdisserendi ratio cognitioque naturae; "
270 DATA "nam de sumtno bono mox, ut dixi, videbimus et ad "
271 DATA "id explicandum disputationem oninem conferemus. "
272 DATA "In his igitur pnrtibus duabus nihil erat quod Zeno "
273 DATA "coromutare gestiret; res enim ae praeclare lialjebat, "
274 DATA "et quidem in utraque parte. Quid enim ab antiquis "
275 DATA "ex eo genere quod ad disserenduni valet praetermis "
276 DATA "Bum est? qui et definierunt plurima et definiendi "
277 DATA "artes reliquerunt, quodque est definitioni adiun "
278 DATA "etum, ut res in partes dividatur, id et fit ab illis et "
279 DATA "quemadmoduni fieri oporleat traditur ; item de c "
280 DATA "trariis, a quibus ad genera formasque generum ven "
281 DATA "runt lam argument! ratione conclusi caput  "
282 DATA "faciunt ea quae perspicua dieunt; deinde ordinc"
283 DATA "sequuntur; tum quid verum sit in singulis extresi "
284 DATA "ponclusio est. Quanta autem ab iUis varietas argu- "
285 DATA "mentorum rati one concludentium eommqiie cum "
286 DATA "captiosis inttrrogationibus dissimilitudo ! Quid quod "
287 DATA "pluribus lotis quasi denuntiunt ut tieque sensuum "
288 DATA "fideni sine ratione nee rationis sine sensibus exqui- "
289 DATA "ranius atqiie ut eorum alterum ab altero ne seps- "
290 DATA "remus? Quid? ea quae diaiectici nunc tradunt el "
291 DATA "docent, nonne ab illis instituta sunt?' De quibus" 
292 DATA "etsi a Clirysippo niaxime est elaboratuni, tanien a "
293 DATA "Zenone minus multo quam ab antiquis ; ab hoc autem "
294 DATA "quaedam non melius quam veteres, quaedam omiiino "
295 DATA "rebcta. Cumque duae sint artes quibus perfecte "
296 DATA "ratio et oratio compleatur, una inveniendi, altera "
297 DATA "hos exilcnij apud illos ubeminam reperiemus. Quam "
298 DATA "multa ab iis conquisita et collects sunt de omnium "
299 DATA "animantiuni genere, ortu, membris, aetatibus quam "
300 DATA "multa de rebus iis quae gignuntur e terra \ quam "
301 DATA "multae quamque de variis rebus et causae cur quid-" 
302 DATA "que at et demonstration es quemadraodum quidque "
303 DATA "fiat! qua e\ omni copia plurima et certissinia argii- "
304 DATA "menta sumuntur ad cuiusque rei naturam explican- "
305 DATA "dam. Ergo adliuc, quantum eqiiidem intellego, "
306 DATA "cautia non videtur fuisse inutandi nominis; non enim, "
307 DATA "si omnia non sequebatur, idcirco non erat ortus "
308 DATA "illinc. Equideni etiam Epicni'um, in physicis quidem, "
309 DATA "Democriteum puto. Pauca mutat, vel plura sane; at "
310 DATA "cum de plurimis eadeni dicit, turn certe de maxi- "
311 DATA "mis. Quod idem cum vestri faciant, non satis ma- "
312 DATA "giiam tribuunt inventoribus gratiam. "
313 DATA "Cum enim superiores, e quibus planissime Polemo, "
314 DATA "secundum naturam vivere summum bouum esse "
315 DATA "dixissent, his verbis tria significari Stoici dicunt, "
316 DATA "unum eiusmodi, vivere adhibentem scientiam earum "
317 DATA "reram quae natura evenirent ; himc ipsum Zenonis "
318 DATA "aiunt esse finem, declarantem illud quod a te dictum "
319 DATA "est, convenienter naturae vivere. Alteram signifi- "
320 DATA "cari idem ut si diceretur officia media omnia aut "
321 DATA "pleraque servantem vivere. Hoc sic expositum dis- "
322 DATA "simile est superiori ; illud enim rectum est (quod "
323 DATA "KaTopSuifia dicebas) contingitque sapienti soli; hoc" 
324 DATA "autera inchoati cuiusdam offici est, non perfecti, quod "
325 DATA "cadere in nonnullos insipientes potest, Tertium "
326 DATA "autein, omnibus aut maximis rebus iis quae secun-" 
327 DATA "dum naturam sint fruentem vivere. Hoc non est "
328 DATA "positum in nostra actione ; completur enim et ex eo "
329 DATA "genere vitae quod virtute fruitor et es iis rebus "
330 DATA "quae sunt secundum naturam nequc sunt in nostra "
331 DATA "potestate. Sed lioc summum bonuni quod tertia "
332 DATA "significatione intellegitur, eaque vita quae ex summo "
333 DATA "bono degitur, quia coniuncta ei virtus est, in sapien- "
334 DATA "tem solum cadit, isque finis bonorum, ut ab ipsis "
335 DATA "Stoicis scriptum videmus, a Xenocrate atque ab "
336 DATA "Aristotele constitutus est. Itaque ab iis constitutio "
337 DATA "ilia prima naturae a qua tu quoque ordiebare I "
338 DATA "prope verbis exponitur. "
339 DATA "Onuiis natura vult esse conservatris sui, ut "
340 DATA "ct salva sit et in genere conservetur suo. Ad banc "
341 DATA "rem aiunt artes quoque requisitas quae naturam "
342 DATA "adtuvarent, in quibus ea numeretur in primis quae "
343 DATA "est Vivendi ars, ut tueatur quod a natura datum sit, "
344 DATA "quod desit acquirat ; iidemque diviserunt naturam "
345 DATA "homintij in animum et corpus ; eumque eorum utrum- "
346 DATA "que per se CKpetendum esse dixisseut, virtutes quo- "
347 DATA "que utriusque eorum perse expetendasesse dieebant; "
348 DATA "et cum aniruum infinita quadam laude anteponerent "
349 DATA "corpori, virtutes quoque animi bonis corporis ante-" 
350 DATA "ponebant. Sed cum sapientiam totlus liominis CU- "
351 DATA "et procuratricem esse vellent, quae esset "
352 DATA "et adiutrix, boc sapientiae munus esse "
353 DATA "am eum tueretur qui constaret ex "
354 DATA "animo et corpore, in utroque luvaret eum ac conti- "
355 DATA "neret. Atque ita re sinipliciter primo collocata, "
356 DATA "reliqua subtilius persequentes corporis bona facilem "
357 DATA "quandam rationem habere censebant, de animi bonis "
358 DATA "accuratius exquirebant, in primisque reperiebant esse "
359 DATA "in iis iustitiae seraina, primique ex omnibus philo- "
360 DATA "sophis natura tributum esse docuerunt ut ii qui pro- "
361 DATA "creati essent a procreatoribus amarentur, et, id quod "
362 DATA "temporum ordine autiquius est, ut coniugia virorum "
363 DATA "et uxorum natura coniuncta esse diccrent, qua e "
364 DATA "stirpe orirentur amicitiae cognationum. Atque ab " 
365 DATA "liis initiis profecti omnium virtutum et originem et "
366 DATA "c'onstaremus et corpore, et haec ipsa et eorum vir- "
367 DATA "tutes per se esse sumendas. An vtro displieuit ea "
368 DATA "quae tributa est animi virtutibus tanta praestantia? "
369 DATA "an quae de prudentia, de cognitione reriim, de con- "
370 DATA "iunctione generis humani, quaeque ab iisdem de "
371 DATA "temperantia, de modestia, de magnitudine animi, de "
372 DATA "omni honestate dicuntur? Fatebuntur Stoici haec "
373 DATA "omnia dicta esse praeclare neqoe earn causam Zenoni "
374 DATA "desciscendi fuisse. Alia qiiaedani dicent, credo, "
375 DATA "magna antiquorum esse peccata quae ille veri inve- "
376 DATA "stigandi ciipidus nullo niodo ferre potuerit. Quid "
377 DATA "enim perversius, quid intolerabilius, quid stultius "
378 DATA "quatn bonam valetudinem, quam dulorum omnium "
379 DATA "vacuitatem, quam integrttatem oi-ulorum reliquorum- "
380 DATA "que sensuum ponere in bonis potius quam dicerent "
381 DATA "nihil omnino inter eas res iisque contrarias interesse? "
382 DATA "ea enim omnia quae illi bona dicerent praeposita "
383 DATA "esse, non bona; itemque il!a quae in corpore excel- "
384 DATA "lerent atulte antiquoa disisse per se esse espetenda; "
385 DATA "sumenda potitis quam expetenda; ea denique omni "
386 DATA "vita quae in virtute una consisleret, illam vitam quae "
387 DATA "etiam ceteris rebus quae essent secundum tiaturam "
388 DATA "abundaret, magis expetendam non esse sed rangis "
389 DATA "sumendam; cumqiie ipsa virtus efficiat ita beatam "
390 DATA "vitam ut beatior esse non possit, tamen quaedam "
391 DATA "deesse sapientibus turn cum sint beatissimi ; itaque "
392 DATA "eos id agere ut a se dolores, morbos, debilitates "
393 DATA "repellant "
394 DATA "O magnam vim ingeni causamque iustam cur "
395 DATA "nova exsisteret disciplina! Pergcporro: sequuntur "
396 DATA "enim ea quae tu scientissime complexus es, omnium "
397 DATA "insipientinni, iniustitiam, alia vitia similia esse, om- "
398 DATA "niaque peccata esse paria, eosque qui natura doctri- "
399 DATA "naque longe ad virtutem processissent, nisi earn "
400 DATA "plane consecuti essent, sumrne esse miseros, neque "
401 DATA "inter eorura vitani et improbissimorum quidquam "
402 DATA "omnino interesse, ut Plato, tantus ille vir, si sapiens "
403 DATA "non fiierit, nihilo melius quam quivis improbissimus "
404 DATA "nee beatius vixerit. Haec videlicet est correctio "
405 DATA "philosopbiae veteris et emendatio, quae omnino "
406 DATA "aditum nullum Iiabere potest in urbem, in fonnrjj in "
407 DATA "curiam. Quis enim ferre posset ita loquentem eum "
408 DATA "qui se auctorem vitae graviter et sapienter aiendae "
409 DATA "profiteretur, nomina rerum commutantem,' cumque "
410 DATA "idem sentiret quod omnes, quibus rebus eandem "
411 DATA "vim tribueret alia nomina imponentem, verba niodo "
412 DATA "sapientia relictae sint. Quod si non hominis sum- "
413 DATA "mum bonum quaereremus sed cuiusdam animaritiSj "
414 DATA "is autem esset nihil nisi animus (liceat enim fingere "
415 DATA "aliquid eiusmodi quo verum facilius reperiamus), "
416 DATA "tamen illi animo non esset hie vester finis. Deside- "
417 DATA "raret enim valetudincm, vacuitatem doloris, appeteret "
418 DATA "etiam conservation em sui earumque rerum custodiam, "
419 DATA "finemque sibi constitueret secundum naturam vivere, "
420 DATA "quod est ut dixi habere ea quae secundum naturam "
421 DATA "Sin dicunt' obscurari quaedam nee apparere "
422 DATA "quia valde parva sint, nos quoque concediTnus; quod "
423 DATA "dicit Epicurus etiam de voluptute, quae minimae "
424 DATA "sint voluptates, eas obseurari saepe et obrui ; sed non "
425 DATA "sunt in eo genere tantae commoditates corporis "
426 DATA "tamque productae tennporibus tamque multae. Ita- "
427 DATA "que in qui bus propter eorum exiguilateni obscnratio "
428 DATA "conseqiiitur, saepe accidit ut nihil interesse nostra "
429 DATA "fateamur sint ilia necne sint (ut in sole, quod a tc "
430 DATA "dicebatur, lucernam adhibcre nihil interest aut "
431 DATA "tentias afferunt ut summura bonuin in eo genere "
432 DATA "ponant quod sit extra nostram potestatem, taraquam "
433 DATA "de inaninio aliquo' loquantur, alii contra, quasi "
434 DATA "corpus nullum sit hominisj ita praeter animuni nihil "
435 DATA "curant, cum praesertim ipse quoque . "
436 DATA "inane nescio quid sit (neque enim "
437 DATA "legere) sed in quodam genere corporis, ut oe is "
438 DATA "quidem viilute una contentus sit sed appetat vacui- "
439 DATA "tatem doloris. Quamobrem utrique idem faciunt ut "
440 DATA "si laevam partem neglegerent, dexteram tuerentur, "
441 DATA "aut ipsius animi, ut fecit Erillus, cognitionem ample- "
442 DATA "xarentur, actionem relin que rent. Eorum enim om- "
443 DATA "nium, multa praetermittentium dum eligant aliquid "
444 DATA "quod sequantur, quasi curta sententia ; at vera iUa" 
445 DATA "perfecta atque plena eorum qui, cum de hominis "
446 DATA "summo bono quaererent, nullam in eo neque animi "
447 DATA "neque corporis partem vacuam tutela reliquerunt. "
448 DATA "Vos aotem, Cato, quia virtus, ut omnes falemur, "
449 DATA "altissimum locum in homine et maxime excellentem "
450 DATA "tenet et quod eos qui sapientes sunt absolutos et "
451 DATA "perfectos putamus, aciem animomm nostrorum "
452 DATA "virtutis splendore praestringitis. In omni enim "
453 DATA "animante est summum aliqutd atque optimum, ut in "
454 DATA "equis, in canibus, quibus tamen et dolore vacare "
455 DATA "opus est et valere "
456 DATA "*"
