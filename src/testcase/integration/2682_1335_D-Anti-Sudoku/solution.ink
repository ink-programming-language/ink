// Translated from solution.cpp.

func fastscan(number: dynamic)
{
  var negative = false;
  var c: dynamic;
  number = 0;
  c = getchar();
  if ((c == cpp_char("-")))
  {
    negative = true;
    c = getchar();
  }
  {
    while ((((c > 47) && (c < 58))))
    {
      number = (((number * 10) + c) - 48);
      c = getchar();
    }
  }
  if (negative)
  {
    number *= -1;
  }
}

func Findsol()
{
  var s = cpp_array(9);
  {
    var i = 0;
    while ((i < 9))
    {
      getline(cin, s[i]);
      i += 1;
    }
  }
  var c1 = s[0].at(8);
  var c2 = s[0].at(7);
  {
    var i = 0;
    while ((i < 9))
    {
      var ind = (((i / 3) + (3 * ((i % 3)))));
      if ((s[i].at(ind) != c1))
      {
        s[i].at(ind) = c1;
      } else
      {
        s[i].at(ind) = c2;
      }
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < 9))
    {
      write(s[i], "\n");
      i += 1;
    }
  }
}

func main()
{
  var t: dynamic;
  fastscan(t);
  while ((cpp_update(t, "--") > 0))
  {
    Findsol();
  }
  return 0;
}
