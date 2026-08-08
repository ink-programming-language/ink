// Translated from solution.cpp.

var s = cpp_array(505);

var P: dynamic;

var N: dynamic;

func main()
{
  var id = 0;
  var pos = 0;
  var neg = 0;
  while ((cin >> s[id]))
  {
    if ((s[id] == "="))
    {
      break;
    }
    if ((s[id] == "?"))
    {
      if (((id == 0) || (s[(id - 1)] == "+")))
      {
        pos += 1;
      } else
      {
        neg += 1;
      }
    }
    id += 1;
    read(s[id]);
    if ((s[id] == "="))
    {
      break;
    }
    id += 1;
  }
  var n: dynamic;
  read(n);
  var dif = (pos - neg);
  {
    var i = 0;
    while ((i < pos))
    {
      P.push_back(1);
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < neg))
    {
      N.push_back(1);
      i += 1;
    }
  }
  if ((dif < n))
  {
    var i = 0;
    while (((dif < n) && (i < pos)))
    {
      if ((((dif + n) - 1) < n))
      {
        dif += (n - 1);
        P[cpp_update(i, "++")] = n;
      } else
      {
        P[cpp_update(i, "++")] = ((1 + n) - dif);
        dif = n;
      }
    }
    if ((dif != n))
    {
      write("Impossible", "\n");
      return 0;
    }
  } else if ((dif > n))
  {
    var i = 0;
    while (((dif > n) && (i < neg)))
    {
      if ((((dif - n) + 1) > n))
      {
        dif -= (n - 1);
        N[cpp_update(i, "++")] = n;
      } else
      {
        N[cpp_update(i, "++")] = ((1 + dif) - n);
        dif = n;
      }
    }
    if ((dif != n))
    {
      write("Impossible", "\n");
      return 0;
    }
  }
  write("Possible", "\n");
  {
    var i = 0;
    var j = 0;
    var k = 0;
    while ((i <= id))
    {
      if ((s[i] == "?"))
      {
        if (((i == 0) || (s[(i - 1)] == "+")))
        {
          write(P[cpp_update(j, "++")]);
        } else
        {
          write(N[cpp_update(k, "++")]);
        }
      } else
      {
        write(" ", s[i]);
      }
      i += 1;
    }
  }
  write(" ", n, "\n");
  return 0;
}
