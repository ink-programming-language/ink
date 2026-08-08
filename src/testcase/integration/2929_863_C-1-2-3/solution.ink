// Translated from solution.cpp.

var a = cpp_array(10, 10);

var b = cpp_array(10, 10);

var viz = cpp_array(10, 10);

var rasp = cpp_array(10, 10);

var sa = cpp_array(10, 10);

var sb = cpp_array(10, 10);

func scor(x: dynamic, y: dynamic)
{
  if ((x == 3))
  {
    if ((y == 3))
    {
      return 0;
    }
    if ((y == 2))
    {
      return 1;
    }
    if ((y == 1))
    {
      return -1;
    }
  }
  if ((x == 2))
  {
    if ((y == 2))
    {
      return 0;
    }
    if ((y == 1))
    {
      return 1;
    }
    if ((y == 3))
    {
      return -1;
    }
  }
  if ((x == 1))
  {
    if ((y == 1))
    {
      return 0;
    }
    if ((y == 3))
    {
      return 1;
    }
    if ((y == 2))
    {
      return -1;
    }
  }
}

func main()
{
  var t: dynamic;
  var a1: dynamic;
  var b1: dynamic;
  var i: dynamic;
  var j: dynamic;
  var scora = 0;
  var scorb = 0;
  var k: dynamic;
  var lasta: dynamic;
  var lastb: dynamic;
  var cnt = 1;
  var tempa: dynamic;
  var tempb: dynamic;
  var s1: dynamic;
  var s2: dynamic;
  read(t, a1, b1);
  {
    i = 1;
    while ((i <= 3))
    {
      {
        j = 1;
        while ((j <= 3))
        {
          read(a[i][j]);
          j += 1;
        }
      }
      i += 1;
    }
  }
  {
    i = 1;
    while ((i <= 3))
    {
      {
        j = 1;
        while ((j <= 3))
        {
          read(b[i][j]);
          j += 1;
        }
      }
      i += 1;
    }
  }
  if ((scor(a1, b1) == 1))
  {
    scora += 1;
  } else if ((scor(a1, b1) == -1))
  {
    scorb += 1;
  }
  lasta = a1;
  lastb = b1;
  sa[a1][b1] = 0;
  sb[a1][b1] = 0;
  viz[a1][b1] = 1;
  rasp[a1][b1] = 1;
  while ((cnt < t))
  {
    tempa = a[lasta][lastb];
    tempb = b[lasta][lastb];
    lasta = tempa;
    lastb = tempb;
    if (viz[tempa][tempb])
    {
      break;
    }
    cnt += 1;
    sa[tempa][tempb] = scora;
    sb[tempa][tempb] = scorb;
    rasp[tempa][tempb] = cnt;
    viz[tempa][tempb] = cnt;
    if ((scor(tempa, tempb) == 1))
    {
      scora += 1;
    } else if ((scor(tempa, tempb) == -1))
    {
      scorb += 1;
    }
  }
  if ((cnt == t))
  {
    write(scora, cpp_char(" "), scorb);
    return 0;
  }
  k = (((t - cnt)) / (((cnt - rasp[tempa][tempb]) + 1)));
  scora = (scora + (((scora - sa[tempa][tempb])) * k));
  scorb = (scorb + (((scorb - sb[tempa][tempb])) * k));
  cnt = (t - (((t - cnt)) % (((cnt - rasp[tempa][tempb]) + 1))));
  lasta = tempa;
  lastb = tempb;
  while ((cnt < t))
  {
    cnt += 1;
    if ((scor(tempa, tempb) == 1))
    {
      scora += 1;
    } else if ((scor(tempa, tempb) == -1))
    {
      scorb += 1;
    }
    tempa = a[lasta][lastb];
    tempb = b[lasta][lastb];
    lasta = tempa;
    lastb = tempb;
  }
  write(scora, cpp_char(" "), scorb);
  return 0;
}
