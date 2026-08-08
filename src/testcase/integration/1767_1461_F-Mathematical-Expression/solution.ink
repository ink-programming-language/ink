// Translated from solution.cpp.

var add = cpp_expression("#include");

var m_p = cpp_expression("#include");

var m_t = cpp_expression("#include <");

var fr = cpp_expression("#incl");

var sc = cpp_expression("#inclu");

var endl = cpp_expression("#inc");

var print = cpp_expression("#include <unordered_set> #include <unorde");

var N = 100005;

var mod = 1000000007;

var M = 505;

var inf = 1000000000000000000;

var rng = cpp_construct(chrono.steady_clock.now().time_since_epoch().count());

var n: dynamic;

var m: dynamic;

var k: dynamic;

var i: dynamic;

var j: dynamic;

var kaskad = cpp_array(N);

var dp = cpp_array(N);

var ans: dynamic;

var x: dynamic;

var sum: dynamic;

var t: dynamic;

var ind: dynamic;

func solve(left: dynamic, right: dynamic)
{
  if ((left == right))
  {
    return;
  }
  while (((left < right) && (kaskad[left] == 1)))
  {
    if ((left < (n - 1)))
    {
      ans[left] = cpp_char("+");
    }
    left += 1;
  }
  right -= 1;
  while (((right >= left) && (kaskad[right] == 1)))
  {
    if ((right >= 1))
    {
      ans[(right - 1)] = cpp_char("+");
    }
    right -= 1;
  }
  right += 1;
  if ((left >= right))
  {
    return;
  }
  t = 1;
  {
    var i = left;
    while ((i < right))
    {
      t *= kaskad[i];
      if ((t > mod))
      {
        t = mod;
      }
      i += 1;
    }
  }
  if ((t == mod))
  {
    return;
  }
  var vec: dynamic;
  {
    var i = left;
    while ((i < right))
    {
      if ((kaskad[i] > 1))
      {
        if (((vec.size() % 2) == 0))
        {
          vec.add(kaskad[i]);
          i += 1;
          continue;
        }
        vec.back() *= kaskad[i];
        i += 1;
        continue;
      }
      if (((vec.size() % 2) == 1))
      {
        vec.add(1);
        i += 1;
        continue;
      }
      vec.back() += 1;
      i += 1;
    }
  }
  var dp = cpp_construct((((vec.size() + 1)) / 2), -1);
  var ch = cpp_construct((((vec.size() + 1)) / 2));
  dp[0] = vec[0];
  {
    var i = 2;
    while ((i < vec.size()))
    {
      x = 1;
      ind = i;
      {
        var j = i;
        while ((j >= 0))
        {
          x *= vec[j];
          sum = x;
          if ((j > 0))
          {
            sum += (dp[(((j - 2)) / 2)] + vec[(j - 1)]);
          }
          if ((sum > dp[(i / 2)]))
          {
            ind = j;
            dp[(i / 2)] = sum;
          }
          j -= 2;
        }
      }
      ch[(i / 2)] = "";
      if ((ind > 0))
      {
        ch[(i / 2)] += ch[(((ind - 2)) / 2)];
        ch[(i / 2)] += cpp_char("+");
      }
      ch[(i / 2)] += string_cpp((((i - ind)) / 2), cpp_char("*"));
      i += 2;
    }
  }
  ind = 0;
  {
    var i = left;
    while ((i < right))
    {
      if ((kaskad[i] == 1))
      {
        while (((i < right) && (kaskad[i] == 1)))
        {
          if ((i > 0))
          {
            ans[(i - 1)] = ch.back()[ind];
          }
          if ((i < (n - 1)))
          {
            ans[i] = ch.back()[ind];
          }
          i += 1;
        }
        ind += 1;
      }
      i += 1;
    }
  }
}

func main(argc: dynamic, argv: dynamic)
{
  ios_base.sync_with_stdio(false);
  cin.tie(0);
  cout.tie(0);
  read(n);
  ans = string_cpp((n - 1), cpp_char("*"));
  {
    var i = 0;
    while ((i < n))
    {
      read(kaskad[i]);
      i += 1;
    }
  }
  var s: dynamic;
  read(s);
  sort(s.begin(), s.end());
  if ((s[0] == cpp_char("+")))
  {
    s = "+";
  }
  if (((s.size() == 1) || (n == 1)))
  {
    write(kaskad[0]);
    {
      var i = 1;
      while ((i < n))
      {
        write(s[0], kaskad[i]);
        i += 1;
      }
    }
    write("\n");
    return 0;
  }
  if ((s.size() == 3))
  {
    s = "*+";
  }
  if ((s == "*-"))
  {
    write(kaskad[0]);
    var flag = ((kaskad[0] == 0));
    {
      var i = 1;
      while ((i < n))
      {
        if (flag)
        {
          write(cpp_char("*"), kaskad[i]);
          i += 1;
          continue;
        }
        if ((kaskad[i] == 0))
        {
          flag = true;
          write(cpp_char("-"));
        } else
        {
          write(cpp_char("*"));
        }
        write(kaskad[i]);
        i += 1;
      }
    }
    write("\n");
    return 0;
  }
  var zeros = [-1];
  {
    var i = 0;
    while ((i < n))
    {
      if ((kaskad[i] == 0))
      {
        if (((i - 1) >= 0))
        {
          ans[(i - 1)] = cpp_char("+");
        }
        if ((i < (n - 1)))
        {
          ans[i] = cpp_char("+");
        }
        zeros.add(i);
      }
      i += 1;
    }
  }
  zeros.add(n);
  {
    var i = 0;
    while ((i < (zeros.size() - 1)))
    {
      solve((zeros[i] + 1), zeros[(i + 1)]);
      i += 1;
    }
  }
  write(kaskad[0]);
  {
    var i = 1;
    while ((i < n))
    {
      write(ans[(i - 1)], kaskad[i]);
      i += 1;
    }
  }
  write("\n");
  write("\n");
}
