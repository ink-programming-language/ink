// Translated from solution.cpp.

var maxn = 3;

var eps = 1e-12;

var n: dynamic;

var ted: dynamic;

var bad: dynamic;

var x = cpp_array(maxn);

var y = cpp_array(maxn);

var r = cpp_array(maxn);

var dis: dynamic;

var tmp: dynamic;

var a: dynamic;

var b: dynamic;

var A: dynamic;

var B: dynamic;

var C: dynamic;

var t = cpp_array(maxn);

var fin = cpp_array(maxn);

var v: dynamic;

func gdis(x1: dynamic, y1: dynamic, x2: dynamic, y2: dynamic)
{
  return sqrt(((((x1 - x2)) * ((x1 - x2))) + (((y1 - y2)) * ((y1 - y2)))));
}

func eq(x1: dynamic, y1: dynamic, x2: dynamic, y2: dynamic)
{
  if (((abs((x1 - x2)) < eps) && (abs((y1 - y2)) < eps)))
  {
    return 1;
  }
  return 0;
}

func main()
{
  read(n);
  {
    var i = 0;
    while ((i < n))
    {
      read(x[i], y[i], r[i]);
      {
        var j = 0;
        while ((j < i))
        {
          dis = gdis(x[i], y[i], x[j], y[j]);
          if (((abs(((dis - r[i]) - r[j])) < eps) || (abs(((max(r[i], r[j]) - min(r[i], r[j])) - dis)) < eps)))
          {
            if ((abs((y[i] - y[j])) < eps))
            {
              a = ((((((r[j] * r[j]) - (r[i] * r[i]))) - (((x[j] * x[j]) - (x[i] * x[i]))))) / ((((x[i] - x[j])) * 2)));
              t[i].push_back([a, y[i]]);
              t[j].push_back([a, y[i]]);
              j += 1;
              continue;
            }
            a = (((x[j] - x[i])) / ((y[i] - y[j])));
            b = (((((((x[i] * x[i]) - (x[j] * x[j]))) + (((y[i] * y[i]) - (y[j] * y[j])))) - (((r[i] * r[i]) - (r[j] * r[j]))))) / ((((y[i] - y[j])) * 2)));
            A = ((a * a) + 1);
            B = ((((a * b) * 2) - (x[i] * 2)) - ((a * y[i]) * 2));
            C = (((((x[i] * x[i]) + (b * b)) + (y[i] * y[i])) - ((b * y[i]) * 2)) - (r[i] * r[i]));
            t[i].push_back([((-B) / ((A * 2))), ((a * (((-B) / ((A * 2))))) + b)]);
            t[j].push_back([((-B) / ((A * 2))), ((a * (((-B) / ((A * 2))))) + b)]);
          } else if ((((r[i] + r[j]) > dis) && (dis > (max(r[i], r[j]) - min(r[i], r[j])))))
          {
            if ((abs((y[i] - y[j])) < eps))
            {
              a = ((((((r[j] * r[j]) - (r[i] * r[i]))) - (((x[j] * x[j]) - (x[i] * x[i]))))) / ((((x[i] - x[j])) * 2)));
              b = sqrt((((r[i] * r[i])) - (((a - x[i])) * ((a - x[i])))));
              t[i].push_back([a, (y[i] - b)]);
              t[j].push_back([a, (y[i] - b)]);
              t[i].push_back([a, (y[i] + b)]);
              t[j].push_back([a, (y[i] + b)]);
              j += 1;
              continue;
            }
            a = (((x[j] - x[i])) / ((y[i] - y[j])));
            b = (((((((x[i] * x[i]) - (x[j] * x[j]))) + (((y[i] * y[i]) - (y[j] * y[j])))) - (((r[i] * r[i]) - (r[j] * r[j]))))) / ((((y[i] - y[j])) * 2)));
            A = ((a * a) + 1);
            B = ((((a * b) * 2) - (x[i] * 2)) - ((a * y[i]) * 2));
            C = (((((x[i] * x[i]) + (b * b)) + (y[i] * y[i])) - ((b * y[i]) * 2)) - (r[i] * r[i]));
            t[i].push_back([((((-B) - sqrt(((B * B) - ((A * C) * 4))))) / ((A * 2))), ((a * (((((-B) - sqrt(((B * B) - ((A * C) * 4))))) / ((A * 2))))) + b)]);
            t[j].push_back([((((-B) - sqrt(((B * B) - ((A * C) * 4))))) / ((A * 2))), ((a * (((((-B) - sqrt(((B * B) - ((A * C) * 4))))) / ((A * 2))))) + b)]);
            t[i].push_back([((((-B) + sqrt(((B * B) - ((A * C) * 4))))) / ((A * 2))), ((a * (((((-B) + sqrt(((B * B) - ((A * C) * 4))))) / ((A * 2))))) + b)]);
            t[j].push_back([((((-B) + sqrt(((B * B) - ((A * C) * 4))))) / ((A * 2))), ((a * (((((-B) + sqrt(((B * B) - ((A * C) * 4))))) / ((A * 2))))) + b)]);
          } else
          {
            j += 1;
            continue;
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < n))
    {
      {
        var j = 0;
        while ((j < ((int_cpp(t[i].size())))))
        {
          bad = 0;
          {
            var k = 0;
            while ((k < ((int_cpp(fin[i].size())))))
            {
              if (eq(t[i][j].first, t[i][j].second, fin[i][k].first, fin[i][k].second))
              {
                bad = 1;
                break;
              }
              k += 1;
            }
          }
          if ((!bad))
          {
            fin[i].push_back(t[i][j]);
          }
          bad = 0;
          {
            var k = 0;
            while ((k < ((int_cpp(v.size())))))
            {
              if (eq(t[i][j].first, t[i][j].second, v[k].first, v[k].second))
              {
                bad = 1;
                break;
              }
              k += 1;
            }
          }
          if ((!bad))
          {
            v.push_back(t[i][j]);
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  bad = 0;
  {
    var i = 0;
    while ((i < n))
    {
      if ((((int_cpp(fin[i].size()))) > 0))
      {
        bad = 1;
        break;
      }
      i += 1;
    }
  }
  if ((!bad))
  {
    write((n + 1));
    return 0;
  }
  bad = 0;
  {
    var i = 0;
    while ((i < n))
    {
      if ((((int_cpp(fin[i].size()))) == 0))
      {
        bad = 1;
        break;
      }
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < n))
    {
      ted += ((int_cpp(fin[i].size())));
      i += 1;
    }
  }
  if (bad)
  {
    write(((ted - ((int_cpp(v.size())))) + 3));
    return 0;
  }
  write(((ted - ((int_cpp(v.size())))) + 2));
  return 0;
}
