// Translated from solution.cpp.

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var q: dynamic = cpp_uninitialized();

var mp: dynamic = cpp_array(2000001);

var R: dynamic = cpp_array(2000001);

var D: dynamic = cpp_array(2000001);

func main() -> dynamic
{
  scanf("%d%d%d", (&n), (&m), (&q));
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      {
        var j: dynamic = 1;
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
    var i: dynamic = 0;
    while ((i <= n))
    {
      {
        var j: dynamic = 0;
        while ((j <= m))
        {
          var pos: dynamic = (((i) * ((m + 1))) + (j));
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
    var a: dynamic = cpp_uninitialized();
    var b: dynamic = cpp_uninitialized();
    var c: dynamic = cpp_uninitialized();
    var d: dynamic = cpp_uninitialized();
    var h: dynamic = cpp_uninitialized();
    var w: dynamic = cpp_uninitialized();
    var t1: dynamic = 0;
    var t2: dynamic = 0;
    var p1: dynamic = cpp_uninitialized();
    var p2: dynamic = cpp_uninitialized();
    scanf("%d%d%d%d%d%d", (&a), (&b), (&c), (&d), (&h), (&w));
    {
      var i: dynamic = 1;
      while ((i < a))
      {
        t1 = D[t1];
        i += 1;
      }
    }
    {
      var i: dynamic = 1;
      while ((i < b))
      {
        t1 = R[t1];
        i += 1;
      }
    }
    {
      var i: dynamic = 1;
      while ((i < c))
      {
        t2 = D[t2];
        i += 1;
      }
    }
    {
      var i: dynamic = 1;
      while ((i < d))
      {
        t2 = R[t2];
        i += 1;
      }
    }
    p1 = t1;
    p2 = t2;
    {
      var i: dynamic = 1;
      while ((i <= h))
      {
        p1 = D[p1];
        p2 = D[p2];
        swap(R[p1], R[p2]);
        i += 1;
      }
    }
    {
      var i: dynamic = 1;
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
      var i: dynamic = 1;
      while ((i <= w))
      {
        p1 = R[p1];
        p2 = R[p2];
        swap(D[p1], D[p2]);
        i += 1;
      }
    }
    {
      var i: dynamic = 1;
      while ((i <= h))
      {
        p1 = D[p1];
        p2 = D[p2];
        swap(R[p1], R[p2]);
        i += 1;
      }
    }
  }
  var pos: dynamic = 1;
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      pos = D[pos];
      var now: dynamic = pos;
      {
        var j: dynamic = 1;
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
