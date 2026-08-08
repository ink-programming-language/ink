// Translated from solution.cpp.

func printTime(sec: dynamic)
{
  printf("%02d:%02d:%02d\n", (sec / 3600), ((sec / 60) % 60), (sec % 60));
}

func main()
{
  while (true)
  {
    var h: dynamic;
    var m: dynamic;
    var s: dynamic;
    scanf("%d %d %d\n", (&h), (&m), (&s));
    if ((((h == -1) && (m == -1)) && (s == -1)))
    {
      break;
    }
    var VideoLen = (2 * 3600);
    var sec = (((h * 3600) + (m * 60)) + s);
    printTime((VideoLen - sec));
    printTime(((VideoLen * 3) - (sec * 3)));
  }
  return 0;
}
