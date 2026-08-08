// Translated from solution.cpp.

func main()
{
  var N: dynamic;
  read(N);
  var num = 0;
  var ans = false;
  {
    var i = 0;
    while ((i < N))
    {
      var d1: dynamic;
      var d2: dynamic;
      read(d1, d2);
      if ((d1 == d2))
      {
        num += 1;
      } else
      {
        num = 0;
      }
      if ((num == 3))
      {
        ans = true;
      }
      i += 1;
    }
  }
  write((if (ans) "Yes" else "No"), "\n");
}
