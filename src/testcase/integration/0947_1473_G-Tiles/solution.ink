// Translated from solution.cpp.

var ll = dynamic;

var p = cpp_expression("#include<bits/");

var endl = cpp_expression("#inc");

var INF = 1000000001;

var C = 998244353;

var fact: dynamic;

var minus_fact: dynamic;

func pow1(x: dynamic, y: dynamic, z: dynamic = C)
{
  if ((y == 0))
  {
    return 1;
  }
  if (((y % 2) == 0))
  {
    return pow1(((x * x) % z), (y / 2), z);
  }
  return ((pow1(x, (y - 1), z) * x) % z);
}

func facts(n: dynamic)
{
  fact = [1];
  minus_fact = [1];
  {
    var q = 1;
    while ((q <= n))
    {
      fact.push_back(((fact.back() * q) % C));
      minus_fact.push_back(((minus_fact.back() * pow1(q, (C - 2))) % C));
      q += 1;
    }
  }
}

func c(k: dynamic, n: dynamic)
{
  if (((k < 0) || (k > n)))
  {
    return 0;
  }
  return ((((fact[n] * minus_fact[k]) % C) * minus_fact[(n - k)]) % C);
}

func main()
{
  ios.sync_with_stdio(false);
  cin.tie(null);
  cout.tie(null);
  facts(200179);
  var n: dynamic;
  read(n);
  {
    var q = 0;
    while ((q < n))
    {
      read(a[q].first, a[q].second);
      q += 1;
    }
  }
  var now = [1];
  {
    var q = 0;
    while ((q < n))
    {
      var w3 = (((((now.size() + a[q].first) + a[q].second) + 1)) / 2);
      var w4 = ((now.size() + a[q].first) + a[q].second);
      var will = cpp_construct((w3 - a[q].second), 0);
      var cc = cpp_construct((now.size() + a[q].first));
      {
        var q1 = 0;
        while ((q1 < cc.size()))
        {
          cc[q1] = c(q1, (a[q].first + a[q].second));
          q1 += 1;
        }
      }
      {
        var q1 = a[q].second;
        while ((q1 < w3))
        {
          var w = min((q1 + 1), cpp_cast(now.size()));
          var w1 = (q1 - a[q].second);
          var w2 = max(0, ((q1 - a[q].first) - a[q].second));
          {
            var q2 = w2;
            while ((q2 < w))
            {
              will[w1] += (cc[(q1 - q2)] * now[q2]);
              q2 += 1;
            }
          }
          q1 += 1;
        }
      }
      now = [];
      for (var q1 in will)
      {
        now.push_back((q1 % C));
      }
      {
        var q1 = ((cpp_cast(now.size()) - 1) - (w4 % 2));
        while ((q1 > -1))
        {
          now.push_back(now[q1]);
          q1 -= 1;
        }
      }
      q += 1;
    }
  }
  var ans = 0;
  for (var q in now)
  {
    ans += q;
  }
  write((ans % C), "\n");
  return 0;
}
