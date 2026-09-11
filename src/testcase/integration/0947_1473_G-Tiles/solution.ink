// Translated from solution.cpp.

var ll: dynamic = dynamic;

var p: dynamic = cpp_expression("#include<bits/");

var endl: dynamic = cpp_expression("#inc");

var INF: dynamic = 1000000001;

var C: dynamic = 998244353;

var fact: dynamic = cpp_uninitialized();

var minus_fact: dynamic = cpp_uninitialized();

func pow1(x: dynamic, y: dynamic, z: dynamic = C) -> dynamic
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

func facts(n: dynamic) -> dynamic
{
  fact = [1];
  minus_fact = [1];
  {
    var q: dynamic = 1;
    while ((q <= n))
    {
      fact.push_back(((fact.back() * q) % C));
      minus_fact.push_back(((minus_fact.back() * pow1(q, (C - 2))) % C));
      q += 1;
    }
  }
}

func c(k: dynamic, n: dynamic) -> dynamic
{
  if (((k < 0) || (k > n)))
  {
    return 0;
  }
  return ((((fact[n] * minus_fact[k]) % C) * minus_fact[(n - k)]) % C);
}

func main() -> dynamic
{
  ios.sync_with_stdio(false);
  cin.tie(null);
  cout.tie(null);
  facts(200179);
  var n: dynamic = cpp_uninitialized();
  read(n);
  {
    var q: dynamic = 0;
    while ((q < n))
    {
      read(a[q].first, a[q].second);
      q += 1;
    }
  }
  var now: dynamic = [1];
  {
    var q: dynamic = 0;
    while ((q < n))
    {
      var w3: dynamic = (((((now.size() + a[q].first) + a[q].second) + 1)) / 2);
      var w4: dynamic = ((now.size() + a[q].first) + a[q].second);
      var will: dynamic = cpp_construct((w3 - a[q].second), 0);
      var cc: dynamic = cpp_construct((now.size() + a[q].first));
      {
        var q1: dynamic = 0;
        while ((q1 < cc.size()))
        {
          cc[q1] = c(q1, (a[q].first + a[q].second));
          q1 += 1;
        }
      }
      {
        var q1: dynamic = a[q].second;
        while ((q1 < w3))
        {
          var w: dynamic = min((q1 + 1), cpp_cast(now.size()));
          var w1: dynamic = (q1 - a[q].second);
          var w2: dynamic = max(0, ((q1 - a[q].first) - a[q].second));
          {
            var q2: dynamic = w2;
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
      for (var q1: dynamic in will)
      {
        now.push_back((q1 % C));
      }
      {
        var q1: dynamic = ((cpp_cast(now.size()) - 1) - (w4 % 2));
        while ((q1 > -1))
        {
          now.push_back(now[q1]);
          q1 -= 1;
        }
      }
      q += 1;
    }
  }
  var ans: dynamic = 0;
  for (var q: dynamic in now)
  {
    ans += q;
  }
  write((ans % C), "\n");
  return 0;
}
