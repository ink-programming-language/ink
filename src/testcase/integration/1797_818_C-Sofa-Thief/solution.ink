// Translated from solution.cpp.

var inf = (1e9 + 7);

var big = ((inf * 1) * inf);

var maxn = 1e6;

var vert = cpp_array(100010);

var hori = cpp_array(100010);

var cntVert = cpp_array(100010);

var cntHori = cpp_array(100010);

var topBefore = cpp_array(100010);

class Sofa
{
  var x1: dynamic;
  var y1: dynamic;
  var x2: dynamic;
  var y2: dynamic;
  func Sofa()
  {
    }
  func Sofa(a: dynamic, b: dynamic, c: dynamic, d: dynamic)
  {
      this->x1 = cpp_construct(a);
      this->y1 = cpp_construct(b);
      this->x2 = cpp_construct(c);
      this->y2 = cpp_construct(d);
    }
  func isVert()
  {
      return (x1 == x2);
    }
  func isHori()
  {
      return (y1 == y2);
    }
}

var sofas = cpp_array(100010);

func main()
{
  ios.sync_with_stdio(0);
  cin.tie(0);
  var d: dynamic;
  read(d);
  var n: dynamic;
  var m: dynamic;
  read(n, m);
  {
    var i = 0;
    while ((i < d))
    {
      var x1: dynamic;
      var y1: dynamic;
      var x2: dynamic;
      var y2: dynamic;
      read(x1, y1, x2, y2);
      sofas[i] = Sofa(x1, y1, x2, y2);
      if ((x1 == x2))
      {
        vert[x1].push_back(i);
        hori[(min(y1, y2))].push_back(i);
        cntVert[x1] += 1;
      } else
      {
        hori[y1].push_back(i);
        vert[min(x1, x2)].push_back(i);
        cntHori[y1] += 1;
      }
      i += 1;
    }
  }
  {
    var row = 1;
    while ((row <= m))
    {
      topBefore[row] = (topBefore[(row - 1)] + hori[row].size());
      row += 1;
    }
  }
  var cntl: dynamic;
  var cntr: dynamic;
  var cntt: dynamic;
  var cntb: dynamic;
  read(cntl, cntr, cntt, cntb);
  var toTheLeft = 0;
  {
    var col = 1;
    while ((col <= n))
    {
      if ((!vert[col].size()))
      {
        col += 1;
        continue;
      }
      for (var id in vert[col])
      {
        var sofa = sofas[id];
        var toTheRight: dynamic;
        var toTheTop: dynamic;
        var toTheBottom: dynamic;
        if (sofa.isVert())
        {
          toTheRight = ((d - toTheLeft) - cntVert[col]);
          toTheTop = (topBefore[min(sofa.y1, sofa.y2)] - 1);
          toTheBottom = ((((d - topBefore[min(sofa.y1, sofa.y2)]) + hori[min(sofa.y1, sofa.y2)].size()) - cntHori[min(sofa.y1, sofa.y2)]) - 1);
          if (cpp_binary(cpp_binary(cpp_binary((toTheLeft == cntl), "and", (toTheRight == cntr)), "and", (toTheTop == cntt)), "and", (toTheBottom == cntb)))
          {
            write((id + 1));
            return 0;
          }
        } else
        {
          toTheRight = (((d - toTheLeft) - cntVert[col]) - 1);
          toTheTop = (topBefore[sofa.y1] - hori[sofa.y1].size());
          toTheBottom = ((d - toTheTop) - cntHori[sofa.y1]);
          if (cpp_binary(cpp_binary(cpp_binary((cntl == ((toTheLeft + vert[col].size()) - 1)), "and", (cntr == toTheRight)), "and", (cntt == toTheTop)), "and", (cntb == toTheBottom)))
          {
            write((id + 1));
            return 0;
          }
        }
      }
      toTheLeft += vert[col].size();
      col += 1;
    }
  }
  write(-1);
  return 0;
}
