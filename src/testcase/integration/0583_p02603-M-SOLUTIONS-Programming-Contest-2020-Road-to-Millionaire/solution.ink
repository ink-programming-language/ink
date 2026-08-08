// Translated from solution.cpp.

func main()
{
  var N: dynamic;
  read(N);
  var yen = 1000;
  var now: dynamic;
  read(now);
  {
    var i = 0;
    while ((i < (N - 1)))
    {
      var nxt: dynamic;
      read(nxt);
      yen += max(0, ((yen / now) * ((nxt - now))));
      now = nxt;
      i += 1;
    }
  }
  write(yen, "\n");
}
