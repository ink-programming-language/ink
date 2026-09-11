// Translated from solution.cpp.

var INF: dynamic = 0x3fffffff;

var N: dynamic = cpp_uninitialized();

var home: dynamic = cpp_array(300);

var gind: dynamic = cpp_array(300);

var gra: dynamic = cpp_array(220);

func main() -> dynamic
{
  var i: dynamic = cpp_uninitialized();
  var j: dynamic = cpp_uninitialized();
  var k: dynamic = cpp_uninitialized();
  var x: dynamic = cpp_uninitialized();
  var y: dynamic = cpp_uninitialized();
  scanf("%d", (&N));
  {
    i = 1;
    while ((i <= N))
    {
      scanf("%d", (&home[i]));
      home[i] -= 1;
      i += 1;
    }
  }
  {
    i = 1;
    while ((i <= N))
    {
      scanf("%d", (&k));
      {
        j = 0;
        while ((j < k))
        {
          scanf("%d", (&y));
          gra[y].push_back(i);
          gind[i] += 1;
          j += 1;
        }
      }
      i += 1;
    }
  }
  var ans: dynamic = INF;
  {
    i = 0;
    while ((i < 3))
    {
      var tmp: dynamic = solve(i);
      if ((tmp < ans))
      {
        ans = tmp;
      }
      i += 1;
    }
  }
  printf("%d\n", ans);
  return 0;
}

func solve(s: dynamic) -> dynamic
{
  var ind: dynamic = cpp_array(300);
  var i: dynamic = cpp_uninitialized();
  var j: dynamic = cpp_uninitialized();
  var k: dynamic = cpp_uninitialized();
  var cnt: dynamic = N;
  var sameroom: dynamic = cpp_uninitialized();
  var res: dynamic = 0;
  var used: dynamic = cpp_array(300);
  memset(used, 0, cpp_sizeof((used)));
  {
    i = 0;
    while ((i < 220))
    {
      ind[i] = gind[i];
      i += 1;
    }
  }
  while ((cnt > 0))
  {
    sameroom = 0;
    {
      i = 1;
      while ((i <= N))
      {
        if ((((!used[i]) && (home[i] == s)) && (ind[i] == 0)))
        {
          {
            j = 0;
            while ((j < gra[i].size()))
            {
              ind[gra[i][j]] -= 1;
              j += 1;
            }
          }
          used[i] = true;
          cnt -= 1;
          sameroom += 1;
        }
        i += 1;
      }
    }
    if ((sameroom == 0))
    {
      s = (((s + 1)) % 3);
      res += 1;
    }
  }
  return (res + N);
}
