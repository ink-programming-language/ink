// Translated from solution.cpp.

var INF = 0x3fffffff;

var N: dynamic;

var home = cpp_array(300);

var gind = cpp_array(300);

var gra = cpp_array(220);

func main()
{
  var i: dynamic;
  var j: dynamic;
  var k: dynamic;
  var x: dynamic;
  var y: dynamic;
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
  var ans = INF;
  {
    i = 0;
    while ((i < 3))
    {
      var tmp = solve(i);
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

func solve(s: dynamic)
{
  var ind = cpp_array(300);
  var i: dynamic;
  var j: dynamic;
  var k: dynamic;
  var cnt = N;
  var sameroom: dynamic;
  var res = 0;
  var used = cpp_array(300);
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
