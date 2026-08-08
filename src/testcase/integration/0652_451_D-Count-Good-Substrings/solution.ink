// Translated from solution.cpp.

var INF = 1e18;

var MOD = (1e9 + 7);

func main()
{
  ios_base.sync_with_stdio(false);
  var s: dynamic;
  read(s);
  var n = s.size();
  var b_odd = 0;
  var b_even = 0;
  var a_odd = 0;
  var a_even = 0;
  var res = 0;
  var reso = 0;
  {
    var i = 0;
    while ((i < n))
    {
      if ((i % 2))
      {
        if ((s[i] == cpp_char("a")))
        {
          a_odd += 1;
          res += a_even;
          reso += a_odd;
        } else
        {
          b_odd += 1;
          res += b_even;
          reso += b_odd;
        }
      } else
      {
        if ((s[i] == cpp_char("a")))
        {
          a_even += 1;
          res += a_odd;
          reso += a_even;
        } else
        {
          b_even += 1;
          res += b_odd;
          reso += b_even;
        }
      }
      i += 1;
    }
  }
  write(res, cpp_char(" "), reso, cpp_char("\n"));
}
