// Translated from solution.cpp.

var x = cpp_array(1234567);

var n: dynamic;

var a: dynamic;

var s: dynamic;

var d: dynamic;

var f: dynamic;

var c: dynamic;

var t: dynamic;

func main()
{
  read(n);
  {
    var i = 0;
    while ((i < n))
    {
      read(x[i]);
      if ((x[i] == 1))
      {
        a += 1;
      } else if ((x[i] == 2))
      {
        s += 1;
      } else if ((x[i] == 3))
      {
        d += 1;
      } else if ((x[i] == 4))
      {
        f += 1;
      }
      i += 1;
    }
  }
  c = f;
  while ((((d != 0) && (a != 0)) && (t != d)))
  {
    if (((a - 1) < 0))
    {
      break;
    }
    a -= 1;
    t += 1;
  }
  c += d;
  t = 0;
  while ((((s != 0) && (a != 0)) && (((t / 2)) != s)))
  {
    if (((a - 2) < 0))
    {
      if (((a - 1) < 0))
      {
        break;
      } else
      {
        a -= 1;
        t += 1;
        break;
      }
    }
    a -= 2;
    t += 2;
  }
  c += (((t / 2)) + ((t % 2)));
  s -= ((((t / 2)) + ((t % 2))));
  if ((s != 0))
  {
    s *= 2;
    c += (((s / 4)) + ((((s % 4)) / 2)));
  }
  if ((a != 0))
  {
    c += ((a / 4));
  }
  if (((a % 4) != 0))
  {
    c += 1;
  }
  write(c);
}
