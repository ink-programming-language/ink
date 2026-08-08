// Translated from solution.cpp.

var caseno = 0;

func yesno(okk: dynamic)
{
  write((if (okk) "YES" else "NO"), cpp_char("\n"));
}

var primemod = 1000000007;

var maxsize = ((1 * 1000000) + 9);

var eps = 1e-10;

var N = 210;

func solve()
{
  var t: dynamic;
  var s: dynamic;
  read(s, t);
  if ((s < t))
  {
    write(s, cpp_char("\n"));
    return;
  }
  var temp = s;
  sort(temp.begin(), temp.end());
  {
    typeof((if (((s.size()) < (t.size()))) (s.size()) else (t.size()))) = ((0) - (((0) > ((if (((s.size()) < (t.size()))) (s.size()) else (t.size()))))));
    while ((i != (((if (((s.size()) < (t.size()))) (s.size()) else (t.size()))) - (((0) > ((if (((s.size()) < (t.size()))) (s.size()) else (t.size()))))))))
    {
      if ((temp[i] < s[i]))
      {
        {
          typeof(s.size()) = (((i + 1)) - ((((i + 1)) > (s.size()))));
          while ((j != ((s.size()) - ((((i + 1)) > (s.size()))))))
          {
            swap(s[i], s[j]);
            if ((s < t))
            {
              cpp_goto("goto h;");
            }
            swap(s[i], s[j]);
            j += (1 - (2 * ((((i + 1)) > (s.size())))));
          }
        }
      }
      i += (1 - (2 * (((0) > ((if (((s.size()) < (t.size()))) (s.size()) else (t.size())))))));
    }
  }
  if ((s < t))
  {
    write(s, cpp_char("\n"));
  } else
  {
    write("---", cpp_char("\n"));
  }
}

func main()
{
  ios_base.sync_with_stdio(false);
  cin.tie(0);
  cout.tie(0);
  var T: dynamic;
  T = 1;
  read(T);
  while (cpp_update(T, "--"))
  {
    solve();
  }
  return 0;
}
