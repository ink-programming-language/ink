// Translated from solution.cpp.

func fast() -> dynamic
{
  ios_base.sync_with_stdio(false);
  cin.tie(null);
}

func vec_splitter(s: dynamic) -> dynamic
{
  s += cpp_char(",");
  var res: dynamic = cpp_uninitialized();
  while ((!s.empty()))
  {
    res.push_back(s.substr(0, s.find(cpp_char(","))));
    s = s.substr((s.find(cpp_char(",")) + 1));
  }
  return res;
}

func debug_out(args: dynamic, idx: dynamic, LINE_NUM: dynamic) -> dynamic
{
  write("\n");
}

func debug_out(args: dynamic, idx: dynamic, LINE_NUM: dynamic, H: dynamic, T: dynamic...) -> dynamic
{
  if ((idx > 0))
  {
    write(", ");
  } else
  {
    write("Line(", LINE_NUM, ") ");
  }
  var ss: dynamic = cpp_uninitialized();
  (ss << H);
  write(args[idx], " = ", ss.str());
  debug_out(args, (idx + 1), LINE_NUM, cpp_expand(T));
}

func get_time() -> dynamic
{
  return ((1.0 * clock()) / CLOCKS_PER_SEC);
}

func main() -> dynamic
{
  fast();
  var n: dynamic = cpp_uninitialized();
  var q: dynamic = cpp_uninitialized();
  read(n, q);
  var s: dynamic = cpp_uninitialized();
  read(s);
  var nxt: dynamic = cpp_construct(26, vector((n + 2), (n + 1)));
  {
    var i: dynamic = (n - 1);
    while ((i >= 0))
    {
      nxt[(s[i] - cpp_char("a"))][i] = i;
      i -= 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < 26))
    {
      {
        var j: dynamic = (n - 1);
        while ((j >= 0))
        {
          nxt[i][j] = min(nxt[i][j], nxt[i][(j + 1)]);
          j -= 1;
        }
      }
      i += 1;
    }
  }
  var dp: dynamic = cpp_construct(256, vector(256, vector(256, (n + 1))));
  dp[0][0][0] = 0;
  var l: dynamic = cpp_construct(3);
  var t: dynamic = cpp_construct(3, "");
  while (cpp_update(q, "--"))
  {
    var ch: dynamic = cpp_uninitialized();
    var c: dynamic = cpp_uninitialized();
    var idx: dynamic = cpp_uninitialized();
    read(ch, idx);
    idx -= 1;
    if ((ch == cpp_char("+")))
    {
      read(c);
      l[idx] += 1;
      t[idx] += c;
    }
    42;
    var lim0: dynamic = ( ((idx == 0)) ? l[0] : 0);
    var lim1: dynamic = ( ((idx == 1)) ? l[1] : 0);
    var lim2: dynamic = ( ((idx == 2)) ? l[2] : 0);
    {
      var i: dynamic = lim0;
      while ((i <= l[0]))
      {
        {
          var j: dynamic = lim1;
          while ((j <= l[1]))
          {
            {
              var k: dynamic = lim2;
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
