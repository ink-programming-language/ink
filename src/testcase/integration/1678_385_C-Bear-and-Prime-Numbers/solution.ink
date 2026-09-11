// Translated from solution.cpp.

var N: dynamic = (1e7 + 5);

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var l: dynamic = cpp_uninitialized();

var t: dynamic = cpp_uninitialized();

var r: dynamic = cpp_uninitialized();

var com: dynamic = cpp_array(N);

var p: dynamic = cpp_array(N);

var freq: dynamic = cpp_array(N);

func seive() -> dynamic
{
  {
    var i: dynamic = 2;
    while ((i < N))
    {
      com[i] += com[(i - 1)];
      if (p[i])
      {
        i += 1;
        continue;
      }
      var num: dynamic = freq[i];
      {
        var x: dynamic = (i + i);
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

func main() -> dynamic
{
  scanf("%d", (&n));
  {
    var i: dynamic = 0;
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
    var i: dynamic = 0;
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
