// Translated from solution.cpp.

var n: dynamic = cpp_uninitialized();

var s: dynamic = cpp_array(100005);

func main(argument_0: dynamic) -> dynamic
{
  read(n);
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      read(s[i]);
      i += 1;
    }
  }
  var ans: dynamic = 0;
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      {
        var j: dynamic = 0;
        while (((j + 2) < s[i].size()))
        {
          if ((s[i].substr(j, 3) == "AOR"))
          {
            ans += 1;
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  var Ocnt: dynamic = 0;
  var A: dynamic = 0;
  var AO: dynamic = 0;
  var R: dynamic = 0;
  var OR: dynamic = 0;
  var R_A: dynamic = 0;
  var OR_A: dynamic = 0;
  var R_AO: dynamic = 0;
  var OR_AO: dynamic = 0;
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      var l: dynamic = s[i].size();
      if ((s[i] == "O"))
      {
        Ocnt += 1;
      }
      if ((s[i].substr((l - 1), 1) == "A"))
      {
        A += 1;
      }
      if (((l >= 2) && (s[i].substr((l - 2), 2) == "AO")))
      {
        AO += 1;
      }
      if ((s[i].substr(0, 1) == "R"))
      {
        R += 1;
      }
      if (((l >= 2) && (s[i].substr(0, 2) == "OR")))
      {
        OR += 1;
      }
      if ((((l >= 2) && (s[i].substr(0, 1) == "R")) && (s[i].substr((l - 1), 1) == "A")))
      {
        R_A += 1;
      }
      if ((((l >= 3) && (s[i].substr(0, 2) == "OR")) && (s[i].substr((l - 1), 1) == "A")))
      {
        OR_A += 1;
      }
      if ((((l >= 3) && (s[i].substr(0, 1) == "R")) && (s[i].substr((l - 2), 2) == "AO")))
      {
        R_AO += 1;
      }
      if ((((l >= 4) && (s[i].substr(0, 2) == "OR")) && (s[i].substr((l - 2), 2) == "AO")))
      {
        OR_AO += 1;
      }
      i += 1;
    }
  }
  var gomi: dynamic = (((((A + AO) + R) + OR) - ((((R_A + OR_A) + R_AO) + OR_AO))) + Ocnt);
  gomi = (n - gomi);
  var mx: dynamic = 0;
  {
    var i: dynamic = 0;
    while ((i <= min(Ocnt, A)))
    {
      var tmp1: dynamic = min((A - i), OR);
      if ((((tmp1 && (OR_A == A)) && (OR_A == OR)) && (i == 0)))
      {
        tmp1 -= 1;
      }
      var tmp2: dynamic = min((AO + i), R);
      if ((((tmp2 && (R_AO == AO)) && (R_AO == R)) && (i == 0)))
      {
        tmp2 -= 1;
      }
      mx = max(mx, (tmp1 + tmp2));
      i += 1;
    }
  }
  if ((mx && (mx == (n - ((Ocnt + gomi))))))
  {
    mx -= 1;
  }
  ans += mx;
  write(ans, "\n");
  return 0;
}
