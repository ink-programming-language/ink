// Translated from solution.cpp.

var rng = cpp_construct(chrono.steady_clock.now().time_since_epoch().count());

func solve()
{
  var S: dynamic;
  var t: dynamic;
  read(S, t);
  var n = S.size();
  var a = [];
  var b = [];
  {
    var i = 0;
    while ((i < n))
    {
      a[(S[i] - cpp_char("0"))][(S[(i + 1)] - cpp_char("0"))] += 1;
      b[(t[i] - cpp_char("0"))][(t[(i + 1)] - cpp_char("0"))] += 1;
      i += 2;
    }
  }
  if (((a[0][0] != b[0][0]) || (a[1][1] != b[1][1])))
  {
    write("-1\n\n");
    return;
  }
  var res: dynamic;
  while (true)
  {
    var s = S;
    res.clear();
    var rev = __cpp_lambda_1;
    {
      var j = (n - 2);
      while ((j >= 0))
      {
        var target = t.substr(j, 2);
        if ((s.substr(j, 2) == target))
        {
          j -= 2;
          continue;
        }
        var i = 0;
        {
          while ((i < j))
          {
            if ((s.substr(i, 2) == target))
            {
              rev((i + 2));
              rev((j + 2));
              break;
            }
            i += 2;
          }
        }
        if ((i >= j))
        {
          i = 0;
          swap(target[0], target[1]);
          {
            while ((i <= j))
            {
              if ((s.substr(i, 2) == target))
              {
                break;
              }
              i += 2;
            }
          }
          assert((i <= j));
          var k = (rng() % (((((j - i)) / 2) + 1)));
          i += (k * 2);
          rev((i + 2));
          j += 2;
        }
        j -= 2;
      }
    }
    assert((s == t));
    if (!(((cpp_cast(res.size()) > (n + 1)))))
    {
      break;
    }
  }
  write(res.size(), "\n");
  for (var x in res)
  {
    write(x, cpp_char(" "));
  }
  write("\n");
}

func main()
{
  ios_base.sync_with_stdio(false);
  cin.tie(null);
  var t: dynamic;
  read(t);
  while (cpp_update(t, "--"))
  {
    solve();
  }
  write("\n");
}

func __cpp_lambda_1(p: dynamic)
{
  if ((!p))
  {
    return;
  }
  res.push_back(p);
  reverse(s.begin(), (s.begin() + p));
}
