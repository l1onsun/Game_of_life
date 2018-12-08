#include <iostream>
//#include <pthread.h>
//#include <string>
#include <algorithm>
#include <vector>
#include <ctime>
#include <atomic>
#include <unistd.h>
//#include <cstdlib>
//#include <utility>

class field {
 public:
  int a[2];
  int b[2];
  void init(int N, int M) {
    a[0] = a[1] = 0;
    b[1] = N;
    b[0] = M;
    findmax();
  }
  field splitedleft() const {
    field newf = field();
    newf.a[maxside] = a[maxside];
    newf.b[maxside] = mean(maxside);
    newf.a[1 - maxside] = a[1 - maxside];
    newf.b[1 - maxside] = b[1 - maxside];
    newf.findmax();
    return newf;
  }
  field splitedright() const {
    field newf = field();
    newf.a[maxside] = mean(maxside);
    newf.b[maxside] = b[maxside];
    newf.a[1 - maxside] = a[1 - maxside];
    newf.b[1 - maxside] = b[1 - maxside];
    newf.findmax();
    return newf;
  }
 private:
  int maxside;
  void findmax() {
    maxside = 0;
    if (b[1] - a[1] > b[0] - a[0])
      maxside = 1;
  }
  int mean(int side) const {
    //return (b[side] - a[side]) / 2;
    return (b[side] + a[side]) / 2;
  }
  /*void split(field* start, field* end) {
    if (end - start < 1) {
      std::cout << "What?" << std::endl;
      return;
    }
    if (end - start == 1) {
      *start = *this;
      return;
    }
    splitedleft().split(start, start + (end-start)/2);
    splitedright().split(start + (end-start)/2, end);
    delete this;
  }*/
};

void split(field f, field* start, field* end) {
  if (end - start < 1) {
    std::cout << "What?" << std::endl;
    return;
  }
  if (end - start == 1) {
    *start = f;
    return;
  }
  split(f.splitedleft(), start, start + (end - start) / 2);
  split(f.splitedright(), start + (end - start) / 2, end);

}

class GameofLife {
 public:
  GameofLife() : pause(false), ending(false) {
    /*
    streams = nullptr;
    fields = nullptr;
    statuses = nullptr;
    N = M = K = -1;
    stoped = pthread_cond_t();
    mutex = pthread_mutex_t();*/
    pthread_cond_init(&stoped, nullptr);
    pthread_mutex_init(&mutex, nullptr);
  }

  void run() {
    printhelp();
    while (true) {
      std::string request;
      std::cin >> request;
      std::transform(request.begin(), request.end(), request.begin(), ::tolower);

      if (request == "start") {
        _start();
      } else if (request == "status") {
        _status();
      } else if (request == "quit" || request == "exit") {
        _quit();
        break;
      } else if (request == "stop") {
        _stop();
      } else if (request == "run") {
        _run();
      } else {
        std::cout << "unknown command" << std::endl;
        printhelp();
      }

    }
  }

 private:
  void _run() {
    int num;
    std::cout << "input the num: ";
    std::cin >> num;
    box.add(num);
    pthread_cond_broadcast(&stoped);
  }

  void _stop() {
    if (!pause) {
      pthread_mutex_lock(&mutex);
      pause = true;
      pthread_mutex_unlock(&mutex);
    } else {
      pthread_mutex_lock(&mutex);
      pause = false;
      pthread_mutex_unlock(&mutex);
    }
  }

  void _quit() {
    pthread_mutex_lock(&mutex);
    pause = false;
    ending = true;
    pthread_mutex_unlock(&mutex);
  }

  void _start() {
    std::cout << "input K: ";
    std::cin >> K;
    std::cout << "input N M: ";
    std::cin >> N >> M;

    init();
    std::cout << "!";
    for (int i = 0; i < K; i++) {
      streams[i].run(i, this);
    }
  }

  void printhelp() {
    std::cout << "hi" << std::endl;
  }

  void _status() {
    //todo
    _stop();
    int min = -1;
    for(int i = 0; i < K; i++) {
      if (statuses[i] < min || min == -1)
        min = statuses[i];
      std::cout << statuses[i] << ' ';
    }
    std::cout << std::endl;
    for(int i = 0; i < N; i++) {
      for (int j = 0; j < M; j++)
        std::cout << box.layers[i][j][min];
      std::cout << std::endl;
    }
  }

  void init() {
    streams = new stream[K];
    fields = new field[K];
    statuses = new int[K];
    for (int i; i < K; i++) {
      statuses[i] = 0;
    }

    box.init(N, M);
    field basefiled = field();
    basefiled.init(N, M);
    split(basefiled, fields, fields + K);
  }

  class stream {
   public:
    void run(int i, GameofLife* game) {
      num = i;
      base = game;
      pthread_create(&thread, nullptr, start_routine, (void*) this);
    }
   private:
    static void* start_routine(void* selfvoid) {
      stream* self = (stream*) selfvoid;
      field f = self->base->fields[self->num];
      while(check(self)) {
        std::cout << '#'  << self->num << std::endl;
        while (self->base->statuses[self->num] >= self->base->box.maxiter.load()) usleep(100);
        std::cout << "start #" << self->num << " iter: " << self->base->statuses[self->num] << std::endl;
        for (int i = f.a[1]; i < f.b[1]; i++)
          for (int j = f.a[0]; j < f.b[0]; j++) {

            int livecount = 0;
            livecount += getcell(self, i - 1, j - 1);
            livecount += getcell(self, i - 1, j);
            livecount += getcell(self, i - 1, j + 1);
            livecount += getcell(self, i, j - 1);
            livecount += getcell(self, i, j + 1);
            livecount += getcell(self, i + 1, j - 1);
            livecount += getcell(self, i + 1, j);
            livecount += getcell(self, i + 1, j + 1);
            if (self->base->box.layers[i][j].back()) {
              if (livecount == 3 || livecount == 2)
                self->base->box.layers[i][j].push_back(true);
              else
                self->base->box.layers[i][j].push_back(false);
            } else {
              if (livecount == 3)
                self->base->box.layers[i][j].push_back(true);
              else
                self->base->box.layers[i][j].push_back(false);
            }
          }
        self->base->statuses[self->num]++;
      }
      return 0;
    }
    static bool check(stream* self) {
      bool ret = true;
      pthread_mutex_lock(&self->base->mutex);
      while (self->base->pause)
        pthread_cond_wait(&self->base->stoped, &self->base->mutex);
      if (self->base->ending)
        ret = false;
      pthread_mutex_unlock(&self->base->mutex);
      return ret;
    }
    static bool getcell(stream* self, int x, int y) {
      while (true) {
        auto p = self->base->box(x - 1, y, self->base->statuses[self->num]);
        if (!p.first) {
          continue;
        } else
          return p.second;
      }
    }

    int num;
    //int iter;
    pthread_t thread;
    GameofLife* base;
  } * streams;
  class ground {
    friend stream;
    friend GameofLife;
   public:
    ground() {
      N = M = 0;
      links = nullptr;
      maxiter.store(0);
    }

    std::pair<bool, bool> operator()(int x, int y, int iter) const {
      if (x < 0 || y < 0 || x >= N || y >= M)
        return std::make_pair(true, false);
      else {
        if (iter < layers[x][y].size())
          return std::make_pair(true, layers[x][y][iter]);
        else
          return std::make_pair(false, false);
      }
    }

    void init(int n, int m) {
      N = n;
      M = m;
      layers = new std::vector<bool>* [N];
      for (int i = 0; i < N + 2; i++)
        layers[i] = new std::vector<bool>[M];
      randomstart();
    }

    void add(int num) {
//      for(int j = 0; j < num; j++) {
//        bool** newlayer = new bool* [N + 2];
//        for (int i = 0; i < N + 2; i++) {
//          newlayer[i] = new bool[M + 2];
//          newlayer[i]++;
//        }
//        newlayer++;
//        for (int i = -1; i <= M; i++)
//          newlayer[-1][i] = newlayer[N][i] = false;
//        for (int i = -1; i <= N; i++)
//          newlayer[i][-1] = newlayer[i][M] = false;
//
//        layers.push_back(newlayer);
      maxiter.fetch_add(num);
    }

   private:

    void randomstart() {
      srand(time(0));
      for (int i = 0; i < N; i++)
        for (int j = 0; j < M; j++)
          layers[i][j].push_back((bool) (rand() % 2));
    }
    std::atomic<int> maxiter;
    int N, M;
    pthread_cond_t*** links;
    std::vector<bool>** layers;
  } box;
  field* fields;
  int* statuses;
  int N, M, K;
  bool pause, ending;
  pthread_mutex_t mutex;
  pthread_cond_t stoped;
};

int main() {
  GameofLife().run();
  return 0;
}
