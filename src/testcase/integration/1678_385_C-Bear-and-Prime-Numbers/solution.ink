// Translated from solution.cpp.

var N = (1e7 + 5);

var n: dynamic;

var m: dynamic;

var l: dynamic;

var t: dynamic;

var r: dynamic;

var com = cpp_array(N);

var p = cpp_array(N);

var freq = cpp_array(N);

func seive()
{
  {
    var i = 2;
    while ((i < N))
    {
      com[i] += com[(i - 1)];
      if (p[i])
      {
        i += 1;
        continue;
      }
      var num = freq[i];
      {
        var x = (i + i);
        while ((x < N))
        {
          num += freq[x];
          p[x] = 1;
          x += i;
        }
      }
      com[i] += num;
      i += 1;
    }
  }
}

func main()
{
  scanf("%d", (&n));
  {
    var i = 0;
    while ((i < n))
    {
      scanf("%d", (&t));
      freq[t] += 1;
      i += 1;
    }
  }
  seive();
  scanf("%d", (&m));
  {
    var i = 0;
    while ((i < m))
    {
      scanf("%d %d", (&l), (&r));
      if ((r > 10000000))
      {
        r = 10000000;
      }
      if ((l > 10000000))
      {
        l = 10000000;
      }
      printf("%lld\n", (com[r] - com[(l - 1)]));
      i += 1;
    }
  }
}
