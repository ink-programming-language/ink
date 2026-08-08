// Translated from solution.cpp.

var N = 200010;

var b = cpp_array(N);

var n: dynamic;

var res = -1;

var mp: dynamic;

func main()
{
  scanf("%d", (&n));
  {
    var i = 1;
    while ((i <= n))
    {
      scanf("%d", (b + i));
      mp[(b[i] - i)] += b[i];
      i += 1;
    }
  }
  {
    var it = mp.begin();
    while ((it != mp.end()))
    {
      if ((res < it->second))
      {
        res = it->second;
      }
      it += 1;
    }
  }
  write(res, "\n");
  return 0;
}
