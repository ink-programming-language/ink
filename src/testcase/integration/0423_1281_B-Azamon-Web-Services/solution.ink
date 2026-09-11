// Translated from solution.cpp.

var caseno: dynamic = 0;

func yesno(okk: dynamic) -> dynamic
{
  write(( (okk) ? "YES" : "NO"), cpp_char("\n"));
}

var primemod: dynamic = 1000000007;

var maxsize: dynamic = ((1 * 1000000) + 9);

var eps: dynamic = 1e-10;

var N: dynamic = 210;

func solve() -> dynamic
{
  var t: dynamic = cpp_uninitialized();
  var s: dynamic = cpp_uninitialized();
  read(s, t);
  if ((s < t))
  {
    write(s, cpp_char("\n"));
    return;
  }
  var temp: dynamic = s;
  sort(temp.begin(), temp.end());
  {
    typeof(( (((s.size()) < (t.size()))) ? (s.size()) : (t.size()))) = ((0) - (((0) > (( (((s.size()) < (t.size()))) ? (s.size()) : (t.size()))))));
    while ((i != ((( (((s.size()) < (t.size()))) ? (s.size()) : (t.size()))) - (((0) > (( (((s.size()) < (t.size()))) ? (s.size()) : (t.size()))))))))
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
      i += (1 - (2 * (((0) > (( (((s.size()) < (t.size()))) ? (s.size()) : (t.size())))))));
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

func main() -> dynamic
{
  ios_base.sync_with_stdio(false);
  cin.tie(0);
  cout.tie(0);
  var T: dynamic = cpp_uninitialized();
  T = 1;
  read(T);
  while (cpp_update(T, "--"))
  {
    solve();
  }
  return 0;
}
