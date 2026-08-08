// Translated from solution.cpp.

var N = (2e5 + 5);

var g = cpp_array(N);

var disconn: dynamic;

var ans_vec: dynamic;

var temp: dynamic;

var temper: dynamic;

func main()
{
  ios_base.sync_with_stdio(false);
  cin.tie(null);
  var n: dynamic;
  var m: dynamic;
  read(n, m);
  var x: dynamic;
  var y: dynamic;
  while (cpp_update(m, "--"))
  {
    read(x, y);
    g[x].insert(y);
    g[y].insert(x);
  }
  var v: dynamic;
  {
    var i = 2;
    while ((i <= n))
    {
      v.push_back(i);
      i += 1;
    }
  }
  var it: dynamic;
  {
    it = g[1].begin();
    while ((it != g[1].end()))
    {
      disconn.insert((*it));
      it += 1;
    }
  }
  var sz = n;
  while ((!v.empty()))
  {
    {
      var i = 0;
      while (((i < v.size()) && (!disconn.empty())))
      {
        x = v[i];
        if ((disconn.find(x) != disconn.end()))
        {
          i += 1;
          continue;
        } else
        {
          {
            it = disconn.begin();
            while ((it != disconn.end()))
            {
              if ((g[x].find((*it)) == g[x].end()))
              {
                temp.push_back((*it));
              }
              it += 1;
            }
          }
          {
            var j = 0;
            while ((j < temp.size()))
            {
              disconn.erase(temp[j]);
              j += 1;
            }
          }
        }
        i += 1;
      }
    }
    {
      var i = 0;
      while ((i < temp.size()))
      {
        x = temp[i];
        {
          it = disconn.begin();
          while ((it != disconn.end()))
          {
            if ((g[x].find((*it)) == g[x].end()))
            {
              temper.push_back((*it));
            }
            it += 1;
          }
        }
        {
          var j = 0;
          while ((j < temper.size()))
          {
            disconn.erase(temper[j]);
            j += 1;
          }
        }
        if ((i == (temp.size() - 1)))
        {
          temp.clear();
          {
            var j = 0;
            while ((j < temper.size()))
            {
              temp.push_back(temper[j]);
              j += 1;
            }
          }
          if ((!temp.empty()))
          {
            i = -1;
          }
          temper.clear();
        }
        i += 1;
      }
    }
    ans_vec.push_back((sz - disconn.size()));
    sz = disconn.size();
    v.clear();
    {
      it = disconn.begin();
      while ((it != disconn.end()))
      {
        v.push_back((*it));
        it += 1;
      }
    }
    it = disconn.begin();
    disconn.erase((*it));
  }
  sort(ans_vec.begin(), ans_vec.end());
  write(ans_vec.size(), "\n");
  {
    var i = 0;
    while ((i < ans_vec.size()))
    {
      write(ans_vec[i], " ");
      i += 1;
    }
  }
  return 0;
}
