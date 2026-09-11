// Translated from solution.cpp.

func main() -> dynamic
{
  ios_base.sync_with_stdio(false);
  var n: dynamic = cpp_uninitialized();
  var i: dynamic = cpp_uninitialized();
  var j: dynamic = cpp_uninitialized();
  var v: dynamic = cpp_uninitialized();
  read(n);
  {
    i = 0;
    while ((i < ((4 * n) + 1)))
    {
      var x: dynamic = cpp_uninitialized();
      var y: dynamic = cpp_uninitialized();
      read(x, y);
      v.push_back(make_pair(x, y));
      i += 1;
    }
  }
  {
    i = 0;
    while ((i < v.size()))
    {
      var aux: dynamic = cpp_uninitialized();
      var f1: dynamic = 0;
      var f2: dynamic = 0;
      var f3: dynamic = 0;
      var f4: dynamic = 0;
      var mx: dynamic = -1;
      var my: dynamic = -1;
      var miy: dynamic = 1e18;
      var mix: dynamic = 1e18;
      {
        j = 0;
        while ((j < v.size()))
        {
          if ((i == j))
          {
            j += 1;
            continue;
          }
          mix = min(mix, v[j].first);
          miy = min(miy, v[j].second);
          mx = max(mx, v[j].first);
          my = max(my, v[j].second);
          aux.push_back(v[j]);
          j += 1;
        }
      }
      {
        j = 0;
        while ((j < aux.size()))
        {
          if ((aux[j].first == mix))
          {
            if (((aux[j].second >= miy) && (aux[j].second <= my)))
            {
              f1 += 1;
            }
          }
          if ((aux[j].first == mx))
          {
            if (((aux[j].second >= miy) && (aux[j].second <= my)))
            {
              f2 += 1;
            }
          }
          if ((aux[j].second == miy))
          {
            if (((aux[j].first >= mix) && (aux[j].first <= mx)))
            {
              f3 += 1;
            }
          }
          if ((aux[j].second == my))
          {
            if (((aux[j].first >= mix) && (aux[j].first <= mx)))
            {
              f4 += 1;
            }
          }
          if (((((aux[j].first != mix) && (aux[j].first != mx)) && (aux[j].second != miy)) && (aux[j].second != my)))
          {
            f1 = -1e18;
          }
          j += 1;
        }
      }
      if (((((f1 >= n) && (f2 >= n)) && (f3 >= n)) && (f4 >= n)))
      {
        write(v[i].first, " ", v[i].second, "\n");
        return 0;
      }
      i += 1;
    }
  }
  return 0;
}
