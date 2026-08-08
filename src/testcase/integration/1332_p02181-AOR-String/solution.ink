// Translated from solution.cpp.

var n: dynamic;

var s = cpp_array(100005);

func main(argument_0: dynamic)
{
  read(n);
  {
    var i = 1;
    while ((i <= n))
    {
      read(s[i]);
      i += 1;
    }
  }
  var ans = 0;
  {
    var i = 1;
    while ((i <= n))
    {
      {
        var j = 0;
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
  var Ocnt = 0;
  var A = 0;
  var AO = 0;
  var R = 0;
  var OR = 0;
  var R_A = 0;
  var OR_A = 0;
  var R_AO = 0;
  var OR_AO = 0;
  {
    var i = 1;
    while ((i <= n))
    {
      var l = s[i].size();
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
  var gomi = (((((A + AO) + R) + OR) - ((((R_A + OR_A) + R_AO) + OR_AO))) + Ocnt);
  gomi = (n - gomi);
  var mx = 0;
  {
    var i = 0;
    while ((i <= min(Ocnt, A)))
    {
      var tmp1 = min((A - i), OR);
      if ((((tmp1 && (OR_A == A)) && (OR_A == OR)) && (i == 0)))
      {
        tmp1 -= 1;
      }
      var tmp2 = min((AO + i), R);
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
