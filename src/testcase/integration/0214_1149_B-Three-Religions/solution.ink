// Translated from solution.cpp.

func fast()
{
  ios_base.sync_with_stdio(false);
  cin.tie(null);
}

func vec_splitter(s: dynamic)
{
  s += cpp_char(",");
  var res: dynamic;
  while ((!s.empty()))
  {
    res.push_back(s.substr(0, s.find(cpp_char(","))));
    s = s.substr((s.find(cpp_char(",")) + 1));
  }
  return res;
}

func debug_out(args: dynamic, idx: dynamic, LINE_NUM: dynamic)
{
  write("\n");
}

func debug_out(args: dynamic, idx: dynamic, LINE_NUM: dynamic, H: dynamic, T: dynamic...)
{
  if ((idx > 0))
  {
    write(", ");
  } else
  {
    write("Line(", LINE_NUM, ") ");
  }
  var ss: dynamic;
  (ss << H);
  write(args[idx], " = ", ss.str());
  debug_out(args, (idx + 1), LINE_NUM, cpp_expand(T));
}

func get_time()
{
  return ((1.0 * clock()) / CLOCKS_PER_SEC);
}

func main()
{
  fast();
  var n: dynamic;
  var q: dynamic;
  read(n, q);
  var s: dynamic;
  read(s);
  var nxt = cpp_construct(26, vector((n + 2), (n + 1)));
  {
    var i = (n - 1);
    while ((i >= 0))
    {
      nxt[(s[i] - cpp_char("a"))][i] = i;
      i -= 1;
    }
  }
  {
    var i = 0;
    while ((i < 26))
    {
      {
        var j = (n - 1);
        while ((j >= 0))
        {
          nxt[i][j] = min(nxt[i][j], nxt[i][(j + 1)]);
          j -= 1;
        }
      }
      i += 1;
    }
  }
  var dp = cpp_construct(256, vector(256, vector(256, (n + 1))));
  dp[0][0][0] = 0;
  var l = cpp_construct(3);
  var t = cpp_construct(3, "");
  while (cpp_update(q, "--"))
  {
    var ch: dynamic;
    var c: dynamic;
    var idx: dynamic;
    read(ch, idx);
    idx -= 1;
    if ((ch == cpp_char("+")))
    {
      read(c);
      l[idx] += 1;
      t[idx] += c;
    }
    42;
    var lim0 = (if ((idx == 0)) l[0] else 0);
    var lim1 = (if ((idx == 1)) l[1] else 0);
    var lim2 = (if ((idx == 2)) l[2] else 0);
    {
      var i = lim0;
      while ((i <= l[0]))
      {
        {
          var j = lim1;
          while ((j <= l[1]))
          {
            {
              var k = lim2;
              while ((k <= l[2]))
              {
                dp[i][j][k] = (n + 1);
                if ((ch == cpp_char("+")))
                {
                  if ((i > 0))
                  {
                    dp[i][j][k] = min(dp[i][j][k], (nxt[(t[0][(i - 1)] - cpp_char("a"))][dp[(i - 1)][j][k]] + 1));
                  }
                  if ((j > 0))
                  {
                    dp[i][j][k] = min(dp[i][j][k], (nxt[(t[1][(j - 1)] - cpp_char("a"))][dp[i][(j - 1)][k]] + 1));
                  }
                  if ((k > 0))
                  {
                    dp[i][j][k] = min(dp[i][j][k], (nxt[(t[2][(k - 1)] - cpp_char("a"))][dp[i][j][(k - 1)]] + 1));
                  }
                }
                k += 1;
              }
            }
            j += 1;
          }
        }
        i += 1;
      }
    }
    if ((ch == cpp_char("-")))
    {
      l[idx] -= 1;
      t[idx] = t[idx].substr(0, (cpp_cast(t[idx].size()) - 1));
    }
    if ((dp[l[0]][l[1]][l[2]] < (n + 1)))
    {
      write("YES", cpp_char("\n"));
    } else
    {
      write("NO", cpp_char("\n"));
    }
  }
}
