// Translated from solution.cpp.

var M: dynamic = (100000 + 10);

var hpos: dynamic = cpp_array(M);

var hneg: dynamic = cpp_array(M);

var criminal: dynamic = cpp_uninitialized();

var claim: dynamic = cpp_array(M);

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var m: dynamic = cpp_uninitialized();
  read(n, m);
  var pos: dynamic = 0;
  var neg: dynamic = 0;
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      var s: dynamic = cpp_uninitialized();
      read(s);
      var num: dynamic = 0;
      {
        var i: dynamic = 1;
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
    var i: dynamic = 1;
    while ((i <= n))
    {
      var num: dynamic = ((hpos[i] + neg) - hneg[i]);
      if ((num == m))
      {
        criminal.insert(i);
      }
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      var id: dynamic = claim[i];
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
