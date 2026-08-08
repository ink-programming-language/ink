// Translated from solution.cpp.

var s = cpp_array(100005);

var day = [31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31];

var my: dynamic;

func check(s: dynamic)
{
  if ((((((((((((((s[0]) >= cpp_char("0")) && ((s[0]) <= cpp_char("9")))) && ((((s[1]) >= cpp_char("0")) && ((s[1]) <= cpp_char("9"))))) && ((((s[3]) >= cpp_char("0")) && ((s[3]) <= cpp_char("9"))))) && ((((s[4]) >= cpp_char("0")) && ((s[4]) <= cpp_char("9"))))) && ((((s[6]) >= cpp_char("0")) && ((s[6]) <= cpp_char("9"))))) && ((((s[7]) >= cpp_char("0")) && ((s[7]) <= cpp_char("9"))))) && ((((s[8]) >= cpp_char("0")) && ((s[8]) <= cpp_char("9"))))) && ((((s[9]) >= cpp_char("0")) && ((s[9]) <= cpp_char("9"))))) && (s[2] == cpp_char("-"))) && (s[5] == cpp_char("-"))))
  {
    var d = atoi(s);
    var m = atoi((s + 3));
    var y = atoi(string_cpp((s + 6), (s + 10)).c_str());
    if ((((((m <= 12) && (d > 0)) && (d <= day[(m - 1)])) && (y >= 2013)) && (y <= 2015)))
    {
      my[string_cpp(s, (s + 10))] += 1;
    }
  }
}

func main()
{
  gets(s);
  var n = strlen(s);
  {
    var i = 0;
    while (((i + 10) <= n))
    {
      check((s + i));
      i += 1;
    }
  }
  var res = cpp_construct("");
  var max = -1;
  {
    typeof((my).begin()) = (my).begin();
    e = (my).end();
    while ((it != e))
    {
      if ((it->second > max))
      {
        max = it->second;
        res = it->first;
      }
      it += 1;
    }
  }
  write(res, "\n");
  return 0;
}
