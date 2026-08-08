// Translated from solution.cpp.

var n: dynamic;

var m: dynamic;

var q: dynamic;

var mp = cpp_array(2000001);

var R = cpp_array(2000001);

var D = cpp_array(2000001);

func main()
{
  scanf("%d%d%d", (&n), (&m), (&q));
  {
    var i = 1;
    while ((i <= n))
    {
      {
        var j = 1;
        while ((j <= m))
        {
          scanf("%d", (&mp[(((i) * ((m + 1))) + (j))]));
          j += 1;
        }
      }
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i <= n))
    {
      {
        var j = 0;
        while ((j <= m))
        {
          var pos = (((i) * ((m + 1))) + (j));
          R[pos] = (((i) * ((m + 1))) + ((j + 1)));
          D[pos] = ((((i + 1)) * ((m + 1))) + (j));
          j += 1;
        }
      }
      i += 1;
    }
  }
  while (cpp_update(q, "--"))
  {
    var a: dynamic;
    var b: dynamic;
    var c: dynamic;
    var d: dynamic;
    var h: dynamic;
    var w: dynamic;
    var t1 = 0;
    var t2 = 0;
    var p1: dynamic;
    var p2: dynamic;
    scanf("%d%d%d%d%d%d", (&a), (&b), (&c), (&d), (&h), (&w));
    {
      var i = 1;
      while ((i < a))
      {
        t1 = D[t1];
        i += 1;
      }
    }
    {
      var i = 1;
      while ((i < b))
      {
        t1 = R[t1];
        i += 1;
      }
    }
    {
      var i = 1;
      while ((i < c))
      {
        t2 = D[t2];
        i += 1;
      }
    }
    {
      var i = 1;
      while ((i < d))
      {
        t2 = R[t2];
        i += 1;
      }
    }
    p1 = t1;
    p2 = t2;
    {
      var i = 1;
      while ((i <= h))
      {
        p1 = D[p1];
        p2 = D[p2];
        swap(R[p1], R[p2]);
        i += 1;
      }
    }
    {
      var i = 1;
      while ((i <= w))
      {
        p1 = R[p1];
        p2 = R[p2];
        swap(D[p1], D[p2]);
        i += 1;
      }
    }
    p1 = t1;
    p2 = t2;
    {
      var i = 1;
      while ((i <= w))
      {
        p1 = R[p1];
        p2 = R[p2];
        swap(D[p1], D[p2]);
        i += 1;
      }
    }
    {
      var i = 1;
      while ((i <= h))
      {
        p1 = D[p1];
        p2 = D[p2];
        swap(R[p1], R[p2]);
        i += 1;
      }
    }
  }
  var pos = 1;
  {
    var i = 1;
    while ((i <= n))
    {
      pos = D[pos];
      var now = pos;
      {
        var j = 1;
        while ((j <= m))
        {
          printf("%d ", mp[now]);
          now = R[now];
          j += 1;
        }
      }
      printf("\n");
      i += 1;
    }
  }
  return 0;
}
