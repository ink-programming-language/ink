// Translated from solution.cpp.

var pref: dynamic = cpp_uninitialized();

var k: dynamic = cpp_uninitialized();

var a: dynamic = cpp_uninitialized();

var n: dynamic = cpp_uninitialized();

func count(cur: dynamic, j: dynamic, i: dynamic) -> dynamic
{
  if ((k[i] > cur))
  {
    var x: dynamic = ((k[i] - cur) + pref[j]);
    j = (lower_bound(pref.begin(), pref.end(), x) - pref.begin());
    if ((j != n))
    {
      cur = (pref[j] - x);
    } else
    {
      cur = 0;
      j = (n - 1);
    }
  } else
  {
    cur -= k[i];
  }
  if ((cur == 0))
  {
    j = (((j + 1)) % n);
    cur = a[j];
  }
  return (n - j);
}

func main() -> dynamic
{
  var q: dynamic = cpp_uninitialized();
  scanf("%d %d", (&n), (&q));
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      var x: dynamic = cpp_uninitialized();
      scanf("%d", (&x));
      a.push_back(x);
      i += 1;
    }
  }
  var j: dynamic = 0;
  var cur: dynamic = a[j];
  pref.push_back(a[0]);
  {
    var i: dynamic = 1;
    while ((i < n))
    {
      pref.push_back((pref[(i - 1)] + a[i]));
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < q))
    {
      var k_i: dynamic = cpp_uninitialized();
      scanf("%lld", (&k_i));
      k.push_back(k_i);
      printf("%d\n", count(cur, j, i));
      i += 1;
    }
  }
  return 0;
}
