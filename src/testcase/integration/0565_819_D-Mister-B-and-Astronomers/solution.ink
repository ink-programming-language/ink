// Translated from solution.cpp.

var size = ((200 * 1000) + 100);

var ans = cpp_array(size);

var a = cpp_array(size);

var mycur = cpp_array(size);

var t: dynamic;

var n: dynamic;

var d: dynamic;

var shift = cpp_array(size);

var ord = cpp_array(size);

func nod(a: dynamic, b: dynamic)
{
  if ((b == 0))
  {
    return a;
  } else
  {
    return nod(b, (a % b));
  }
}

func pwr(a: dynamic, b: dynamic, mdl: dynamic)
{
  if ((b == 0))
  {
    return (1 % mdl);
  }
  var d = pwr(a, (b / 2), mdl);
  d = ((((d * 1) * d)) % mdl);
  if ((b & 1))
  {
    d = ((((d * 1) * a)) % mdl);
  }
  return d;
}

func factor(val: dynamic)
{
  var res: dynamic;
  {
    var i = 2;
    while (((i * i) <= val))
    {
      if (((val % i) == 0))
      {
        res[i] = 0;
        while (((val % i) == 0))
        {
          res[i] += 1;
          val /= i;
        }
      }
      i += 1;
    }
  }
  if ((val > 1))
  {
    res[val] = 1;
  }
  return res;
}

func main()
{
  scanf("%d%d", (&t), (&n));
  {
    var i = 0;
    while ((i < n))
    {
      scanf("%d", (&a[i]));
      i += 1;
    }
  }
  var hps: dynamic;
  var cur = 0;
  {
    var i = 0;
    while ((i < n))
    {
      mycur[i] = cur;
      if (hps.count(mycur[i]))
      {
        ans[i] = 0;
      } else
      {
        ans[i] = 1;
      }
      hps.insert(mycur[i]);
      cur += a[(((i + 1)) % n)];
      cur %= t;
      i += 1;
    }
  }
  d = nod(t, cur);
  var fact = factor((t / d));
  var phi = 1;
  for (var e in fact)
  {
    {
      var i = 0;
      while ((i < (cpp_cast(e.second) - 1)))
      {
        phi *= e.first;
        i += 1;
      }
    }
    phi *= ((e.first - 1));
  }
  var back = pwr((cur / d), (phi - 1), (t / d));
  {
    var i = 0;
    while ((i < n))
    {
      if (((ans[i] == 0) || (cur == 0)))
      {
        i += 1;
        continue;
      }
      shift[i] = (mycur[i] % d);
      ord[i] = ((((((mycur[i] / d)) * 1) * back)) % ((t / d)));
      i += 1;
    }
  }
  if ((cur > 0))
  {
    var ords: dynamic;
    {
      var i = 0;
      while ((i < n))
      {
        if ((ans[i] > 0))
        {
          ords[shift[i]].push_back(make_pair(ord[i], i));
        }
        i += 1;
      }
    }
    for (var e in ords)
    {
      sort(e.second.begin(), e.second.end());
      {
        var j = 0;
        while ((j < (cpp_cast(e.second.size()) - 1)))
        {
          ans[e.second[j].second] = (e.second[(j + 1)].first - e.second[j].first);
          j += 1;
        }
      }
      ans[e.second.back().second] = (((t / d) - e.second.back().first) + e.second[0].first);
    }
  }
  {
    var i = 0;
    while ((i < n))
    {
      printf("%d%c", ans[i], " \n"[(i == (n - 1))]);
      i += 1;
    }
  }
  return 0;
}
