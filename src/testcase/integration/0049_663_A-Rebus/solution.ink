// Translated from solution.cpp.

var s: dynamic = cpp_array(505);

var P: dynamic = cpp_uninitialized();

var N: dynamic = cpp_uninitialized();

func main() -> dynamic
{
  var id: dynamic = 0;
  var pos: dynamic = 0;
  var neg: dynamic = 0;
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
  var n: dynamic = cpp_uninitialized();
  read(n);
  var dif: dynamic = (pos - neg);
  {
    var i: dynamic = 0;
    while ((i < pos))
    {
      P.push_back(1);
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < neg))
    {
      N.push_back(1);
      i += 1;
    }
  }
  if ((dif < n))
  {
    var i: dynamic = 0;
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
    var i: dynamic = 0;
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
    var i: dynamic = 0;
    var j: dynamic = 0;
    var k: dynamic = 0;
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
