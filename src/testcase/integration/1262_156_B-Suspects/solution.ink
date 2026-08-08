// Translated from solution.cpp.

var M = (100000 + 10);

var hpos = cpp_array(M);

var hneg = cpp_array(M);

var criminal: dynamic;

var claim = cpp_array(M);

func main()
{
  var n: dynamic;
  var m: dynamic;
  read(n, m);
  var pos = 0;
  var neg = 0;
  {
    var i = 1;
    while ((i <= n))
    {
      var s: dynamic;
      read(s);
      var num = 0;
      {
        var i = 1;
        while ((i < s.length()))
        {
          num = (((num * 10) + s[i]) - cpp_char("0"));
          i += 1;
        }
      }
      if ((s[0] == cpp_char("+")))
      {
        hpos[num] += 1;
        pos += 1;
        claim[i] = num;
      } else
      {
        hneg[num] += 1;
        neg += 1;
        claim[i] = (-num);
      }
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= n))
    {
      var num = ((hpos[i] + neg) - hneg[i]);
      if ((num == m))
      {
        criminal.insert(i);
      }
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= n))
    {
      var id = claim[i];
      if ((id > 0))
      {
        if ((criminal.find(id) == criminal.end()))
        {
          write("Lie", "\n");
        } else
        {
          if ((criminal.size() == 1))
          {
            write("Truth", "\n");
          } else
          {
            write("Not defined", "\n");
          }
        }
      } else
      {
        id = (-id);
        if ((criminal.find(id) == criminal.end()))
        {
          write("Truth", "\n");
        } else
        {
          if ((criminal.size() == 1))
          {
            write("Lie", "\n");
          } else
          {
            write("Not defined", "\n");
          }
        }
      }
      i += 1;
    }
  }
  return 0;
}
