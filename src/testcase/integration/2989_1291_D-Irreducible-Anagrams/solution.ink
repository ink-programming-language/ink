// Translated from solution.cpp.

var TAM = (2e5 + 2);

var q: dynamic;

var a: dynamic;

var b: dynamic;

var s: dynamic;

var acum = cpp_array(30, TAM);

func carry_on(pos: dynamic)
{
  {
    var j = 0;
    while ((j < 26))
    {
      acum[pos][j] = acum[(pos - 1)][j];
      j += 1;
    }
  }
}

func check(a: dynamic, b: dynamic)
{
  var cont = 0;
  {
    var j = 0;
    while ((j < 26))
    {
      if ((acum[b][j] - acum[(a - 1)][j]))
      {
        cont += 1;
      }
      j += 1;
    }
  }
  return (cont >= 3);
}

func main()
{
  read(s);
  {
    var i = 0;
    while ((i < s.size()))
    {
      carry_on((i + 1));
      acum[(i + 1)][(s[i] - cpp_char("a"))] += 1;
      i += 1;
    }
  }
  read(q);
  while (cpp_update(q, "--"))
  {
    read(a, b);
    if (cpp_binary(cpp_binary((a == b), "or", (s[(a - 1)] != s[(b - 1)])), "or", check(a, b)))
    {
      write("Yes\n");
    } else
    {
      write("No\n");
    }
  }
  return 0;
}
