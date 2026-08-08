// Translated from solution.cpp.

var n: dynamic;

var m: dynamic;

var K: dynamic;

var Q: dynamic;

var pv = cpp_array(101000);

var IT = cpp_array(((131072 + 131072) + 1));

var E = cpp_array(101000);

var chk = cpp_array(201000);

class point
{
  var x: dynamic;
  var y: dynamic;
  func operator_less(p: dynamic)
  {
      return (y < p.y);
    }
}

var w = cpp_array(201000);

class Query
{
  var x1: dynamic;
  var x2: dynamic;
  var y1: dynamic;
  var y2: dynamic;
  var num: dynamic;
  func operator_less(p: dynamic)
  {
      return (y1 < p.y1);
    }
}

var P = cpp_array(201000);

func Push(x: dynamic, y: dynamic)
{
  x += 131072;
  IT[x] = y;
  while ((x != 1))
  {
    x >>= 1;
    IT[x] = max(IT[(x * 2)], IT[((x * 2) + 1)]);
  }
}

func Max(b: dynamic, e: dynamic)
{
  b += 131072;
  e += 131072;
  var r = 0;
  while ((b <= e))
  {
    r = max(r, IT[b]);
    r = max(r, IT[e]);
    b = (((b + 1)) >> 1);
    e = (((e - 1)) >> 1);
  }
  return r;
}

func Do()
{
  var i: dynamic;
  var pv2 = 1;
  {
    i = 1;
    while ((i <= 100000))
    {
      E[i].clear();
      pv[i] = 0;
      Push(i, 101000);
      i += 1;
    }
  }
  sort((w + 1), ((w + K) + 1));
  sort((P + 1), ((P + Q) + 1));
  {
    i = 1;
    while ((i <= K))
    {
      if ((!E[w[i].x].size()))
      {
        Push(w[i].x, w[i].y);
      }
      E[w[i].x].push_back(w[i].y);
      i += 1;
    }
  }
  {
    i = 1;
    while ((i <= Q))
    {
      while (((pv2 <= K) && (w[pv2].y < P[i].y1)))
      {
        pv[w[pv2].x] += 1;
        if ((pv[w[pv2].x] == E[w[pv2].x].size()))
        {
          Push(w[pv2].x, 101000);
        } else
        {
          Push(w[pv2].x, E[w[pv2].x][pv[w[pv2].x]]);
        }
        pv2 += 1;
      }
      if ((Max(P[i].x1, P[i].x2) <= P[i].y2))
      {
        chk[P[i].num] = true;
      }
      i += 1;
    }
  }
}

func main()
{
  var i: dynamic;
  scanf("%d%d%d%d", (&n), (&m), (&K), (&Q));
  {
    i = 1;
    while ((i <= K))
    {
      scanf("%d%d", (&w[i].x), (&w[i].y));
      i += 1;
    }
  }
  {
    i = 1;
    while ((i <= Q))
    {
      scanf("%d%d%d%d", (&P[i].x1), (&P[i].y1), (&P[i].x2), (&P[i].y2));
      P[i].num = i;
      i += 1;
    }
  }
  Do();
  {
    i = 1;
    while ((i <= K))
    {
      swap(w[i].x, w[i].y);
      i += 1;
    }
  }
  {
    i = 1;
    while ((i <= Q))
    {
      swap(P[i].x1, P[i].y1);
      swap(P[i].x2, P[i].y2);
      i += 1;
    }
  }
  Do();
  {
    i = 1;
    while ((i <= Q))
    {
      if (chk[i])
      {
        printf("YES\n");
      } else
      {
        printf("NO\n");
      }
      i += 1;
    }
  }
}
