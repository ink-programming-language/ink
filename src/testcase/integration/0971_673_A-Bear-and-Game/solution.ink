// Translated from solution.cpp.

func main()
{
  var arr = [];
  var n: dynamic;
  scanf("%d", (&n));
  {
    var i = 1;
    while ((i <= n))
    {
      var m: dynamic;
      scanf("%d", (&m));
      arr[m] = 1;
      i += 1;
    }
  }
  var c = 0;
  var ans = 0;
  var i: dynamic;
  {
    i = 1;
    while ((i <= 90))
    {
      c += 1;
      if ((arr[i] != 0))
      {
        c = 0;
      }
      if ((c == 15))
      {
        break;
      }
      i += 1;
    }
  }
  if ((i == 91))
  {
    i -= 1;
  }
  write(i);
}
