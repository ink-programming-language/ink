// Translated from solution.cpp.

var pref: dynamic;

var k: dynamic;

var a: dynamic;

var n: dynamic;

func count(cur: dynamic, j: dynamic, i: dynamic)
{
  if ((k[i] > cur))
  {
    var x = ((k[i] - cur) + pref[j]);
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

func main()
{
  var q: dynamic;
  scanf("%d %d", (&n), (&q));
  {
    var i = 0;
    while ((i < n))
    {
      var x: dynamic;
      scanf("%d", (&x));
      a.push_back(x);
      i += 1;
    }
  }
  var j = 0;
  var cur = a[j];
  pref.push_back(a[0]);
  {
    var i = 1;
    while ((i < n))
    {
      pref.push_back((pref[(i - 1)] + a[i]));
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < q))
    {
      var k_i: dynamic;
      scanf("%lld", (&k_i));
      k.push_back(k_i);
      printf("%d\n", count(cur, j, i));
      i += 1;
    }
  }
  return 0;
}
