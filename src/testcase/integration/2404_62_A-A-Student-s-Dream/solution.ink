// Translated from solution.cpp.

func main()
{
  var ml: dynamic;
  var mr: dynamic;
  var hl: dynamic;
  var hr: dynamic;
  read(ml, mr, hl, hr);
  var ok = 0;
  if ((((hr + 1) >= ml) && (hr <= (2 * ((ml + 1))))))
  {
    ok = 1;
  }
  if ((((hl + 1) >= mr) && (hl <= (2 * ((mr + 1))))))
  {
    ok = 1;
  }
  printf(if (ok) "YES\n" else "NO\n");
  return 0;
}
