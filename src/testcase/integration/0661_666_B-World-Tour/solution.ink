// Translated from solution.cpp.

var u = cpp_array(3005);

var sp = cpp_array(3005, 3005);

var n: dynamic;

var m: dynamic;

var fin = cpp_array(3005);

var fout = cpp_array(3005);

var p = cpp_array(4);

var bp = cpp_array(4);

var bl: dynamic;

func upd()
{
  var l = 0;
  {
    var i = (0);
    while ((i < (3)))
    {
      l += sp[p[i]][p[(i + 1)]];
      i += 1;
    }
  }
  if ((l > bl))
  {
    {
      var i = (1);
      while ((i < (4)))
      {
        {
          var j = (0);
          while ((j < (i)))
          {
            if ((p[i] == p[j]))
            {
              return;
            }
            j += 1;
          }
        }
        i += 1;
      }
    }
    {
      var i = (0);
      while ((i < (4)))
      {
        bp[i] = p[i];
        i += 1;
      }
    }
    bl = l;
  }
}

var q = cpp_array(3005);

var qs: dynamic;

var qe: dynamic;

func main()
{
  scanf("%d%d", (&n), (&m));
  {
    var z = (0);
    while ((z < (m)))
    {
      var x: dynamic;
      var y: dynamic;
      scanf("%d%d", (&x), (&y));
      x -= 1;
      y -= 1;
      u[x].push_back(y);
      z += 1;
    }
  }
  {
    var i = (0);
    while ((i < (n)))
    {
      {
        var j = (0);
        while ((j < (n)))
        {
          sp[i][j] = 1e5;
          j += 1;
        }
      }
      qs = cpp_assign(qe, "=", 0);
      q[cpp_update(qe, "++")] = i;
      sp[i][i] = 0;
      while ((qs != qe))
      {
        var f = q[cpp_update(qs, "++")];
        for (var j in u[f])
        {
          if ((sp[i][j] > 1e4))
          {
            sp[i][j] = (sp[i][f] + 1);
            q[cpp_update(qe, "++")] = j;
          }
        }
      }
      i += 1;
    }
  }
  {
    var i = (0);
    while ((i < (n)))
    {
      {
        var j = (0);
        while ((j < (n)))
        {
          if (((j != i) && (sp[j][i] < 1e4)))
          {
            fin[i].emplace_back(sp[j][i], j);
            if ((int_cpp(fin[i].size()) > 3))
            {
              sort(fin[i].begin(), fin[i].end());
              fin[i].erase(fin[i].begin());
            }
          }
          j += 1;
        }
      }
      {
        var j = (0);
        while ((j < (n)))
        {
          if (((j != i) && (sp[i][j] < 1e4)))
          {
            fout[i].emplace_back(sp[i][j], j);
            if ((int_cpp(fout[i].size()) > 3))
            {
              sort(fout[i].begin(), fout[i].end());
              fout[i].erase(fout[i].begin());
            }
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  {
    var i = (0);
    while ((i < (n)))
    {
      {
        var j = (0);
        while ((j < (n)))
        {
          if (((i != j) && (sp[i][j] < 1e4)))
          {
            p[1] = i;
            p[2] = j;
            for (var f in fin[i])
            {
              for (var e in fout[j])
              {
                p[0] = f.second;
                p[3] = e.second;
                upd();
              }
            }
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  {
    var i = (0);
    while ((i < (4)))
    {
      printf("%d%c", (1 + bp[i]), " \n"[(i == 3)]);
      i += 1;
    }
  }
}
