// Translated from solution.cpp.

func sci(t: dynamic)
{
  read(t);
}

func sci(t: dynamic, ts: dynamic...)
{
  sci(t);
  sci(cpp_expand(ts));
}

func main()
{
  cin.tie(null);
  cout.tie(null);
  ios_base.sync_with_stdio(false);
  var s: dynamic;
  read(s);
  var ans = 1;
  var kek: dynamic;
  kek[cpp_char("4")] = cpp_char("6");
  kek[cpp_char("6")] = cpp_char("4");
  kek[cpp_char("5")] = cpp_char("9");
  kek[cpp_char("9")] = cpp_char("5");
  kek[cpp_char("8")] = cpp_char("0");
  kek[cpp_char("0")] = cpp_char("8");
  kek[cpp_char("7")] = cpp_char("7");
  kek[cpp_char("3")] = cpp_char("3");
  var n = s.length();
  {
    int64_t(i) = 0;
    while (((i) < cpp_cast((((n / 2) + 1)))))
    {
      if ((((s[i] == cpp_char("1")) || (s[i] == cpp_char("2"))) || (kek[s[i]] != s[((n - i) - 1)])))
      {
        ans = 0;
        break;
      }
      (i) += 1;
    }
  }
  write((if ((ans)) "Yes" else "No"), cpp_char("\n"));
  return 0;
}
